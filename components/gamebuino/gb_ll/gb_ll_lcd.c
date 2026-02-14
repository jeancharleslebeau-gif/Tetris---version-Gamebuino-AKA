/*
===============================================================================
  gb_ll_lcd.cpp — This file is part of the Gamebuino-AKA library,
  Copyright (c) Gamebuino 2026

   LGPL v3+
-------------------------------------------------------------------------------
  Modifications JC :
    - Désactivation complète de USE_VSYCNC (FMARK)
    - Suppression des boucles d’attente FMARK
    - Ajout d’un vTaskDelay(1) dans lcd_refresh()
===============================================================================
*/

#include "gb_common.h"
#include "gb_config.h"
#include "gb_ll_lcd.h"
#include "gb_ll_expander.h"
#include "gb_ll_system.h"

#include <string.h>
#include <esp_lcd_panel_io.h>
#include "esp_lcd_io_i80.h"
#include "esp_lcd_panel_st7789.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"

// ============================================================================
// Types & globals
// ============================================================================
static volatile uint32_t u32_start_refresh = 0;
static volatile uint32_t u32_delta_refresh = 0;
static volatile uint8_t  u8_refresh_ctr = 0;
static volatile uint32_t u32_draw_count = 0;

esp_lcd_i80_bus_handle_t i80_bus = NULL;
esp_lcd_panel_io_handle_t lcd_panel_h;

#ifdef USE_PSRAM_VIDEO_BUFFER
    gb_pixel* framebuffer;
#else
    gb_pixel framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
#endif

// ============================================================================
// Prototypes internes
// ============================================================================
static void lcd_init_pwm(void);
void lcd_update_pwm(uint16_t duty);
static void ILI9342C_write_cmd(uint8_t cmd, const uint8_t *data, int len);
static void st7789v_rotation_set(uint8_t rotation);
static void set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void LCD_FAST_test(const gb_pixel *buf);
static void ILI9342C_hard_reset(void);

// ============================================================================
// Callbacks
// ============================================================================
IRAM_ATTR bool color_trans_done_cb(esp_lcd_panel_io_handle_t panel_io,
                                   esp_lcd_panel_io_event_data_t *edata,
                                   void *user_ctx)
{
    (void)panel_io;
    (void)edata;
    (void)user_ctx;
    u32_delta_refresh = gb_get_millis() - u32_start_refresh;
    u8_refresh_ctr = 1;
    u32_start_refresh = 0;
    return false;
}

// ============================================================================
// Bus I80 config
// ============================================================================
esp_lcd_i80_bus_config_t bus_config = {
    .dc_gpio_num = LCD_PIN_DnC,
    .wr_gpio_num = LCD_PIN_nWR,
    .clk_src = LCD_CLK_SRC_PLL160M,
    .data_gpio_nums = {
        LCD_PIN_DB0, LCD_PIN_DB1, LCD_PIN_DB2, LCD_PIN_DB3,
        LCD_PIN_DB4, LCD_PIN_DB5, LCD_PIN_DB6, LCD_PIN_DB7,
    },
    .bus_width = 8,
    .max_transfer_bytes = 320 * 240 * sizeof(uint16_t),
    .psram_trans_align = 64,
    .sram_trans_align = 64,
};

esp_lcd_panel_io_i80_config_t panel_config = {
    .cs_gpio_num = -1,
    .pclk_hz = 20000000,
    .trans_queue_depth = 320 * 16,
    .on_color_trans_done = color_trans_done_cb,
    .user_ctx = 0,
    .lcd_cmd_bits = 8,
    .lcd_param_bits = 8,
    .dc_levels = {
        .dc_idle_level = 1,
        .dc_cmd_level = 0,
        .dc_dummy_level = 1,
        .dc_data_level = 1
    },
    .flags = {
        .cs_active_high = 0,
        .reverse_color_bits = 0,
        .swap_color_bytes = 1,
        .pclk_active_neg = 0,
        .pclk_idle_low = 0
    }
};

// ============================================================================
// GPIO helpers (version ESP-IDF, C pur)
// ============================================================================
#define OUTPUT 1
#define INPUT  0

static void pinMode(gpio_num_t pin, uint8_t mode)
{
    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << pin,
        .mode = (mode == OUTPUT ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT),
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&cfg);
}

static void digitalWrite(gpio_num_t pin, uint8_t level)
{
    gpio_set_level(pin, level);
}

static int digitalRead(gpio_num_t pin)
{
    return gpio_get_level(pin);
}

// ============================================================================
// PWM LCD (backlight)
// ============================================================================
#define LEDC_TIMER      LEDC_TIMER_0
#define LEDC_MODE       LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO  PWM_LCD_GPIO
#define LEDC_CHANNEL    LEDC_CHANNEL_0
#define LEDC_DUTY_RES   LEDC_TIMER_10_BIT
#define LEDC_FREQUENCY  (PWM_LCD_FREQUENCY)

static void lcd_init_pwm(void)
{
    ledc_timer_config_t ledc_timer = {
        .speed_mode      = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num       = LEDC_TIMER,
        .freq_hz         = LEDC_FREQUENCY,
        .clk_cfg         = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_MODE,
        .channel    = LEDC_CHANNEL,
        .timer_sel  = LEDC_TIMER,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = LEDC_OUTPUT_IO,
        .duty       = 0,
        .hpoint     = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void lcd_update_pwm(uint16_t duty)
{
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
}

// ============================================================================
// Bus init / uninit
// ============================================================================
void LCD_i80_bus_init(void)
{
    printf("esp_lcd_new_i80_bus\n");
    esp_err_t ret = esp_lcd_new_i80_bus(&bus_config, &i80_bus);
    printf("esp_lcd_new_i80_bus return %d\n", ret);
    if (ret) for(;;);

    printf("esp_lcd_new_panel_io_i80\n");
    ret = esp_lcd_new_panel_io_i80(i80_bus, &panel_config, &lcd_panel_h);
    printf("esp_lcd_new_panel_io_i80 return %d\n", ret);
    if (ret) for(;;);
}

void LCD_i80_bus_uninit(void)
{
    esp_lcd_panel_io_del(lcd_panel_h);
    esp_lcd_del_i80_bus(i80_bus);
}

// ============================================================================
// Reset LCD
// ============================================================================
static void ILI9342C_hard_reset(void)
{
    gb_ll_expander_lcd_reset(0);
    gb_delay_ms(10);
    gb_ll_expander_lcd_reset(1);
    gb_delay_ms(100);
#if (BOARD_VERSION < 4)
    expander_lcd_cs(0);
#endif
    gb_delay_ms(100);
}

// ============================================================================
// Commandes LCD bas niveau
// ============================================================================
static void ILI9342C_write_cmd(uint8_t cmd, const uint8_t *data, int len)
{
    esp_lcd_panel_io_tx_param(lcd_panel_h, cmd, data, len);
}

void LCD_FAST_test(const gb_pixel *buf)
{
    u32_start_refresh = gb_get_millis();
    esp_lcd_panel_io_tx_color(lcd_panel_h, ST7789V_CMD_RAMWR,
                              buf, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(gb_pixel_16));
}

// ============================================================================
// Rotation & fenêtre
// ============================================================================
static void st7789v_rotation_set(uint8_t rotation)
{
    uint8_t mad_cfg = 0;
    switch (rotation % 4) {
        case 0:
            mad_cfg = ST7789V_MADCTRL_MX | ST7789V_MADCTRL_MY | ST7789V_MADCTRL_RGB;
            break;
        case 1:
            mad_cfg = ST7789V_MADCTRL_MV | ST7789V_MADCTRL_RGB;
            break;
        case 2:
            mad_cfg = ST7789V_MADCTRL_RGB;
            break;
        case 3:
            mad_cfg = ST7789V_MADCTRL_MX | ST7789V_MADCTRL_MV | ST7789V_MADCTRL_RGB;
            break;
        default:
            break;
    }
    ILI9342C_write_cmd(ST7789V_CMD_MADCTL, &mad_cfg, 1);
}

static void set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint8_t x_coord[4];
    uint8_t y_coord[4];

    x_coord[0] = x0 >> 8;
    x_coord[1] = (uint8_t)x0;
    x_coord[2] = x1 >> 8;
    x_coord[3] = (uint8_t)x1;

    y_coord[0] = y0 >> 8;
    y_coord[1] = (uint8_t)y0;
    y_coord[2] = y1 >> 8;
    y_coord[3] = (uint8_t)y1;

    ILI9342C_write_cmd(ST7789V_CMD_CASET, x_coord, 4);
    ILI9342C_write_cmd(ST7789V_CMD_RASET, y_coord, 4);
}

// ============================================================================
// FPS
// ============================================================================
void lcd_set_fps(uint8_t fps)
{
    if (fps > 100) fps = 100;
    if (fps < 40)  fps = 40;

    uint32_t div = 10000000 / (336 * fps);
    if (div > 250) div -= 250;
    else div = 0;

    uint8_t cfg = div / 16;
    if (cfg > 31) cfg = 31;
    if (cfg < 1)  cfg = 1;

    printf("Sef FPS to %d : %d\n", fps, cfg);
    ILI9342C_write_cmd(0xC6, &cfg, 1);
}

// ============================================================================
// Init LCD haut niveau
// ============================================================================
void gb_ll_lcd_init(void)
{
    lcd_init_pwm();
    lcd_update_pwm(64);

#ifdef USE_PSRAM_VIDEO_BUFFER
    framebuffer = (gb_pixel*)heap_caps_calloc(2 * 320 * 240, sizeof(gb_pixel),
                                              MALLOC_CAP_SPIRAM);
    printf("Alloc ram ptr %p\n", framebuffer);
    while (!framebuffer) {
        printf("Alloc ram FAIL\n");
        gb_delay_ms(1000);
    }
#endif

    pinMode(LCD_FMARK, INPUT);
    gb_ll_expander_lcd_rd(1);

    printf("LCD_i80_bus_init()\n");
    LCD_i80_bus_init();

    ILI9342C_hard_reset();
    ILI9342C_write_cmd(ST7789V_CMD_RESET, 0, 0);
    ILI9342C_write_cmd(ST7789V_CMD_SLPOUT, 0, 0);

    const uint8_t color_mode[] = { COLOR_MODE_65K | COLOR_MODE_16BIT };
    ILI9342C_write_cmd(ST7789V_CMD_COLMOD, color_mode, 1);
    ILI9342C_write_cmd(ST7789V_CMD_INVON, 0, 0);
    ILI9342C_write_cmd(ST7789V_CMD_NORON, 0, 0);

    st7789v_rotation_set(3);
    set_addr_window(0, 0, 319, 239);
    ILI9342C_write_cmd(ST7789V_CMD_DISPON, 0, 0);

    const uint8_t te_cfg[] = { 0x00 };
    ILI9342C_write_cmd(0x35, te_cfg, 1);

    u8_refresh_ctr = 1;
    lcd_set_fps(100);
}

// ============================================================================
// Pixel access
// ============================================================================
void lcd_putpixel(uint16_t x, uint16_t y, gb_pixel c)
{
    uint32_t off = x + y * SCREEN_WIDTH;
    if (off < SCREEN_WIDTH * SCREEN_HEIGHT)
        framebuffer[off] = c;
}

gb_pixel lcd_getpixel(uint16_t x, uint16_t y)
{
    uint32_t off = x + y * SCREEN_WIDTH;
    if (off < SCREEN_WIDTH * SCREEN_HEIGHT)
        return framebuffer[off];
    return 0;
}

// ============================================================================
// Refresh / sync
// ============================================================================
uint8_t lcd_refresh_completed(void)
{
    return u8_refresh_ctr;
}

uint32_t gb_ll_lcd_get_draw_count(void)
{
    return u32_draw_count;
}

void lcd_refresh(void)
{
    while (u8_refresh_ctr == 0);
    u8_refresh_ctr = 0;

    // Pas de FMARK : on laisse juste FreeRTOS respirer un peu
    vTaskDelay(1);

    u32_draw_count++;
    LCD_FAST_test(framebuffer);
}

// ============================================================================
// Effets simples
// ============================================================================
void lcd_clear(uint16_t color)
{
    gb_pixel *p = framebuffer;
    uint32_t count = SCREEN_WIDTH * SCREEN_HEIGHT / 4;
    while (count--) {
        *p++ = color;
        *p++ = color;
        *p++ = color;
        *p++ = color;
    }
}

void lcd_dpo(void)
{
#ifndef USE_VIDEO_256_INDEXED
    gb_pixel *p = framebuffer;
    uint32_t count = SCREEN_WIDTH * SCREEN_HEIGHT;
    while (count--) {
        gb_pixel c = *p;
        c &= 0xF7DF;
        *p++ = c >> 1;
    }
#endif
}

void lcd_scrool_vertical(int16_t scroll_pixels)
{
    if (scroll_pixels > 0) {
        if (scroll_pixels > SCREEN_HEIGHT - 1)
            scroll_pixels = SCREEN_HEIGHT - 1;

        memmove(framebuffer,
                &framebuffer[SCREEN_WIDTH * scroll_pixels],
                SCREEN_WIDTH * (SCREEN_HEIGHT - scroll_pixels) * sizeof(gb_pixel));

        memset(&framebuffer[SCREEN_WIDTH * (SCREEN_HEIGHT - scroll_pixels)],
               0,
               SCREEN_WIDTH * scroll_pixels * sizeof(gb_pixel));
    }
}
