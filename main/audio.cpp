/*
===============================================================================
  audio.cpp — Implémentation du système audio (buzzer PWM) pour Tetris AKA
-------------------------------------------------------------------------------
  Rôle :
    - Initialiser le PWM via LEDC (timer + canal dédiés).
    - Fournir tone(), click(), audio_line_clear(), etc.
    - Ne pas interférer avec :
        * le PWM du LCD (LEDC_TIMER_0 / CHANNEL_0)
        * l’I2C (expander)
        * le LCD i80 (DMA + interruptions)
    - Volume global simple (0..8)
===============================================================================
*/

#include "audio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// -----------------------------------------------------------------------------
//  CONFIG LEDC AUDIO (centralisé)
// -----------------------------------------------------------------------------
#define AUDIO_LEDC_TIMER      LEDC_TIMER_3
#define AUDIO_LEDC_CHANNEL    LEDC_CHANNEL_3
#define AUDIO_LEDC_MODE       LEDC_LOW_SPEED_MODE
#define AUDIO_LEDC_GPIO       4

static int g_volume = 8; // 0..8

// -----------------------------------------------------------------------------
//  Initialisation LEDC
// -----------------------------------------------------------------------------
void audio_init()
{
    ledc_timer_config_t timer = {
        .speed_mode       = AUDIO_LEDC_MODE,
        .duty_resolution  = LEDC_TIMER_10_BIT,
        .timer_num        = AUDIO_LEDC_TIMER,
        .freq_hz          = 2000,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t ch = {
        .gpio_num       = AUDIO_LEDC_GPIO,
        .speed_mode     = AUDIO_LEDC_MODE,
        .channel        = AUDIO_LEDC_CHANNEL,
        .timer_sel      = AUDIO_LEDC_TIMER,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&ch);
}

// -----------------------------------------------------------------------------
//  Volume global
// -----------------------------------------------------------------------------
void audio_set_volume(int v)
{
    g_volume = (v < 0 ? 0 : (v > 8 ? 8 : v));
}

// -----------------------------------------------------------------------------
//  Tone générique
// -----------------------------------------------------------------------------
void tone(int freq, int ms, int duty)
{
    if(freq <= 0 || g_volume == 0)
    {
        ledc_stop(AUDIO_LEDC_MODE, AUDIO_LEDC_CHANNEL, 0);
        return;
    }

    int eff_duty = duty > 0 ? duty : (g_volume * 64);

    ledc_set_freq(AUDIO_LEDC_MODE, AUDIO_LEDC_TIMER, freq);
    ledc_set_duty(AUDIO_LEDC_MODE, AUDIO_LEDC_CHANNEL, eff_duty);
    ledc_update_duty(AUDIO_LEDC_MODE, AUDIO_LEDC_CHANNEL);

    if(ms > 0)
    {
        vTaskDelay(pdMS_TO_TICKS(ms));
        ledc_set_duty(AUDIO_LEDC_MODE, AUDIO_LEDC_CHANNEL, 0);
        ledc_update_duty(AUDIO_LEDC_MODE, AUDIO_LEDC_CHANNEL);
    }
}

// -----------------------------------------------------------------------------
//  Sons utilitaires
// -----------------------------------------------------------------------------
void click()            { tone(1200, 40); }
void audio_hard_drop()  { tone(1500, 50); }

// -----------------------------------------------------------------------------
//  Sons de lignes
// -----------------------------------------------------------------------------
void audio_line_clear(int n)
{
    if(n <= 0) return;

    static const int freq[5] = { 0, 600, 800, 1000, 1200 };
    static const int dur [5] = { 0, 40, 40, 60, 80   };

    tone(freq[n > 4 ? 4 : n], dur[n > 4 ? 4 : n]);
}
