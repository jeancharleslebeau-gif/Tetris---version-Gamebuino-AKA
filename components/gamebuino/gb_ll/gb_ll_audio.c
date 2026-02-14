/*
This file is part of the Gamebuino-AKA library,
Copyright (c) Gamebuino 2026

This is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License (LGPL)
as published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License (LGPL) for more details.

You should have received a copy of the GNU Lesser General Public
License (LGPL) along with the library.
If not, see <http://www.gnu.org/licenses/>.

Authors:
 - Jean-Marie Papillon
*/

#include "gb_common.h"
#include "gb_ll_i2c.h"
#include "gb_ll_expander.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "driver/i2s_std.h"
#include <string.h>
#include "gb_ll_lcd.h"
#include "gb_ll_audio.h"
#include "TAS2505_regs.h"
#include "gb_ll_system.h"

// ============================================================
//  ACTIVE / DESACTIVE L'AUDIO I2S + AMPLI
// ============================================================
//  Pour Tetris : laisser commenté
//  Pour jeux avec musique : décommenter
//
// #define USE_I2S_AUDIO
// ============================================================


#ifdef USE_I2S_AUDIO

// -----------------------------------------------------------------------------
//  Variables internes
// -----------------------------------------------------------------------------
static i2s_chan_handle_t tx_chan;

int16_t i16_fifo[ GB_AUDIO_BUFFER_SAMPLE_COUNT * GB_AUDIO_BUFFER_FIFO_COUNT ];
uint32_t u32_idx_read = 0;
uint32_t u32_idx_write = 0;

uint32_t inline fifo_size() { return count_of(i16_fifo); }
uint32_t inline fifo_used() { return u32_idx_write - u32_idx_read; }
uint32_t inline fifo_free() { return fifo_size() - fifo_used(); }

uint16_t u16_buffer_miss = 0;
uint16_t u16_buffer_overvlw = 0;


// -----------------------------------------------------------------------------
//  Push buffer audio
// -----------------------------------------------------------------------------
void gb_ll_audio_push_buffer(const int16_t* pi16_audio_buffer)
{
    if (fifo_free() >= GB_AUDIO_BUFFER_SAMPLE_COUNT)
    {
        memcpy(&i16_fifo[u32_idx_write % fifo_size()],
               pi16_audio_buffer,
               2 * GB_AUDIO_BUFFER_SAMPLE_COUNT);

        u32_idx_write += GB_AUDIO_BUFFER_SAMPLE_COUNT;
    }
    else
        u16_buffer_overvlw++;
}


// -----------------------------------------------------------------------------
//  Callback I2S
// -----------------------------------------------------------------------------
IRAM_ATTR bool i2s_cb(i2s_chan_handle_t handle, i2s_event_data_t *event, void *user_ctx)
{
    int16_t* pSamples = event->dma_buf;
    uint16_t pWordLength = event->size / 2;

    if (fifo_used() >= pWordLength)
    {
        memcpy(pSamples,
               &i16_fifo[u32_idx_read % fifo_size()],
               2 * pWordLength);

        u32_idx_read += pWordLength;
    }
    else
    {
        u16_buffer_miss++;
        while (pWordLength--)
            *pSamples++ = 0;
    }

    return true;
}

i2s_event_callbacks_t i2s_event_callbacks = {
    .on_recv = 0,
    .on_recv_q_ovf = 0,
    .on_sent = i2s_cb,
    .on_send_q_ovf = 0
};


// -----------------------------------------------------------------------------
//  Init I2S
// -----------------------------------------------------------------------------
int gb_ll_init_i2S()
{
    i2s_chan_config_t tx_chan_cfg = {
        .id = I2S_NUM_AUTO,
        .role = I2S_ROLE_MASTER,
        .dma_desc_num = 2,
        .dma_frame_num = GB_AUDIO_BUFFER_SAMPLE_COUNT,
        .auto_clear_after_cb = false,
        .auto_clear_before_cb = false,
        .allow_pd = false,
        .intr_priority = 3,
    };

    ESP_ERROR_CHECK(i2s_new_channel(&tx_chan_cfg, &tx_chan, NULL));

    i2s_std_config_t tx_std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(GB_AUDIO_SAMPLE_RATE),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,
                                                    I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
#ifdef USE_MCLK_ON_GPIO42
            .mclk = GPIO_NUM_42,
#else
            .mclk = I2S_GPIO_UNUSED,
#endif
            .bclk = I2S_PIN_BCLK,
            .ws   = I2S_PIN_WS,
            .dout = I2S_PIN_DOUT,
            .din  = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };

    tx_std_cfg.slot_cfg.bit_shift = true;

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_chan, &tx_std_cfg));
    i2s_channel_register_event_callback(tx_chan, &i2s_event_callbacks, NULL);
    ESP_ERROR_CHECK(i2s_channel_enable(tx_chan));

    return 0;
}


// -----------------------------------------------------------------------------
//  Init ampli + codec
// -----------------------------------------------------------------------------
int gb_ll_audio_init()
{
    printf("audio_init()\n");

    gb_ll_expander_audio_amplifier_reset(0);
    gb_delay_ms(100);
    gb_ll_expander_audio_amplifier_reset(1);
    gb_delay_ms(100);

    gb_ll_init_i2S();

    // --- Toute la config TAS2505 (inchangée) ---
    gb_ll_audio_amp_write(AUDIO_AMP_REG_PAGE, 0);
    gb_ll_audio_amp_write(AUDIO_AMP_SOFT_RESET, 1);

    gb_ll_audio_amp_write(AUDIO_AMP_REG_PAGE, 1);
    gb_ll_audio_amp_write(AUDIO_AMP_P1_LDO_CTRL, 0);

    gb_ll_audio_amp_write(AUDIO_AMP_REG_PAGE, 0);
#ifdef USE_MCLK_ON_GPIO42
    gb_ll_audio_amp_write(AUDIO_AMP_P0_CLK_SETTING1, 3);
#else
    gb_ll_audio_amp_write(AUDIO_AMP_P0_CLK_SETTING1, 4 + 3);
#endif

#ifdef USE_MCLK_ON_GPIO42
    gb_ll_audio_amp_write(AUDIO_AMP_P0_CLK_SETTING2, 0x91);
#else
    gb_ll_audio_amp_write(AUDIO_AMP_P0_CLK_SETTING2, 0x93);
#endif

    gb_ll_audio_amp_write(6, 20);
    gb_ll_audio_amp_write(7, 0x00);
    gb_ll_audio_amp_write(8, 0x00);
    gb_delay_ms(15);

    gb_ll_audio_amp_write(0x0B, 0x84);
    gb_ll_audio_amp_write(0x0C, 0x82);
    gb_ll_audio_amp_write(0x0D, 0x00);
    gb_ll_audio_amp_write(0x0E, 0x80);

    gb_ll_audio_amp_write(AUDIO_AMP_AIS_REG1, 0x00);
    gb_ll_audio_amp_write(0x1C, 0x00);
    gb_ll_audio_amp_write(0x3C, 0x02);

    gb_ll_audio_amp_write(AUDIO_AMP_REG_PAGE, 0);
    gb_ll_audio_amp_write(0x3F, 0x90);
    gb_ll_audio_amp_write(0x41, 0x00);
    gb_ll_audio_amp_write(0x40, 0x04);

    gb_ll_audio_amp_write(AUDIO_AMP_REG_PAGE, 1);
    gb_ll_audio_amp_write(0x01, 0x10);
    gb_ll_audio_amp_write(0x0A, 0x00);
    gb_ll_audio_amp_write(0x0C, 0x04);
    gb_ll_audio_amp_write(0x16, 0x00);
    gb_ll_audio_amp_write(0x18, 0x00);
    gb_ll_audio_amp_write(0x19, 0x00);
    gb_ll_audio_amp_write(0x09, 0x20);
    gb_ll_audio_amp_write(0x10, 0x00);
    gb_ll_audio_amp_write(AUDIO_AMP_P1_SPK_VOL, 0);
    gb_ll_audio_amp_write(AUDIO_AMP_P1_SPK_AMP_VOL, 0x20);

    gb_ll_audio_amp_write(AUDIO_AMP_REG_PAGE, 1);
    gb_ll_audio_amp_write(AUDIO_AMP_P1_SPK_AMP, 0x02);

    return 0;
}


// -----------------------------------------------------------------------------
//  Volume
// -----------------------------------------------------------------------------
void gb_ll_audio_set_volume(uint8_t u8_volume)
{
    u8_volume = 127 - u8_volume / 2;

    gb_ll_audio_amp_write(AUDIO_AMP_REG_PAGE, 1);

    if (u8_volume >= 116)
    {
        gb_ll_audio_amp_write(AUDIO_AMP_P1_SPK_VOL, 0x7F);
        gb_ll_audio_amp_write(AUDIO_AMP_P1_HP_SPK_VOL, 0x7F);
    }
    else
    {
        gb_ll_audio_amp_write(AUDIO_AMP_P1_SPK_VOL, u8_volume);
        gb_ll_audio_amp_write(AUDIO_AMP_P1_HP_SPK_VOL, u8_volume);
    }
}


// -----------------------------------------------------------------------------
//  Vibrateur
// -----------------------------------------------------------------------------
void gb_ll_audio_set_vibrator(uint8_t u8_on)
{
    gb_ll_audio_amp_write(AUDIO_AMP_REG_PAGE, 0);

    if (u8_on)
        gb_ll_audio_amp_write(AUDIO_AMP_P0_GPIO_CTRL, 0b00001101);
    else
        gb_ll_audio_amp_write(AUDIO_AMP_P0_GPIO_CTRL, 0b00001100);
}

#endif // USE_I2S_AUDIO



// ============================================================
//  STUBS quand l'audio I2S est désactivé
// ============================================================
#ifndef USE_I2S_AUDIO

int gb_ll_audio_init() { return 0; }
void gb_ll_audio_set_volume(uint8_t v) {}
void gb_ll_audio_set_vibrator(uint8_t on) {}

#endif