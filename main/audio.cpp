#define USE_I2S_AUDIO   

/*
===============================================================================
  audio.cpp — Système audio Tetris AKA (I2S + square wave)
===============================================================================
*/

#include "audio.h"
#include "gb_ll_audio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>
#include <string.h>
#include "esp_random.h"
#include "audio_pmf.h"

// -----------------------------------------------------------------------------
// Volume global SFX (0..8)
// -----------------------------------------------------------------------------
static int g_volume = 8;

// Tâche PMF
static TaskHandle_t s_pmf_task = nullptr;

// -----------------------------------------------------------------------------
// Mapping volume SFX (0..8) -> volume TAS2505 (0..255)
// -----------------------------------------------------------------------------
static inline uint8_t map_sfx_volume(int v)
{
    if (v <= 0) return 0;      // mute
    if (v >= 8) return 255;    // max
    return (uint8_t)(v * 32);  // 0,32,64,...,256
}

// -----------------------------------------------------------------------------
// Tâche PMF
// -----------------------------------------------------------------------------
static void pmf_task(void* arg)
{
    (void)arg;
    while (true)
    {
        audio_pmf_update();           // fait tourner le player PMF
        vTaskDelay(pdMS_TO_TICKS(5)); // toutes les ~5 ms
    }
}

// -----------------------------------------------------------------------------
// Synthèse PCM simple (square + noise)
// -----------------------------------------------------------------------------

static void generate_square(int16_t* buf, int samples, int freq, int sample_rate, int amp)
{
    if (freq <= 0) {
        memset(buf, 0, samples * sizeof(int16_t));
        return;
    }

    float period = (float)sample_rate / (float)freq;
    float phase = 0.0f;

    for (int i = 0; i < samples; ++i) {
        buf[i] = (phase < period / 2.0f) ? amp : -amp;
        phase += 1.0f;
        if (phase >= period) phase -= period;
    }
}

static void generate_noise(int16_t* buf, int samples, int amp)
{
    for (int i = 0; i < samples; ++i) {
        int r = (esp_random() & 0xFFFF) - 32768;
        buf[i] = (int16_t)((r * amp) / 32768);
    }
}

// -----------------------------------------------------------------------------
// Tone simple
// -----------------------------------------------------------------------------
void tone(int freq, int ms, int duty)
{
#ifdef USE_I2S_AUDIO
    if (g_volume <= 0 || ms <= 0) return;

    const int sample_rate = GB_AUDIO_SAMPLE_RATE;
    int total_samples = (sample_rate * ms) / 1000;
    if (total_samples <= 0) return;

    int amp = (g_volume * 3000) / 8;

    int16_t buf[GB_AUDIO_BUFFER_SAMPLE_COUNT];

    while (total_samples > 0) {
        int chunk = total_samples;
        if (chunk > GB_AUDIO_BUFFER_SAMPLE_COUNT)
            chunk = GB_AUDIO_BUFFER_SAMPLE_COUNT;

        generate_square(buf, chunk, freq, sample_rate, amp);
        gb_ll_audio_push_buffer(buf);

        total_samples -= chunk;
    }
#else
    (void)freq; (void)ms; (void)duty;
#endif
}

// -----------------------------------------------------------------------------
// API publique
// -----------------------------------------------------------------------------
void audio_init()
{
    g_volume = 8; // valeur par défaut

#ifdef USE_I2S_AUDIO
    // IMPORTANT : appliquer le volume matériel AVANT le PMF
    uint8_t hw = map_sfx_volume(g_volume);
    gb_ll_audio_set_volume(hw);
#endif

    // Init du player PMF
    audio_pmf_init();

    // Tâche PMF
    if (s_pmf_task == nullptr)
    {
        xTaskCreate(pmf_task, "pmf_audio", 4096, nullptr, 4, &s_pmf_task);
    }
}

void audio_set_volume(int v)
{
    if (v < 0) v = 0;
    if (v > 8) v = 8;
    g_volume = v;

#ifdef USE_I2S_AUDIO
    uint8_t hw = map_sfx_volume(v);
    gb_ll_audio_set_volume(hw);
#endif
}

// -----------------------------------------------------------------------------
// SFX
// -----------------------------------------------------------------------------
void click()               { tone(1200, 40, 0); }
void audio_move()          { tone(900,  30, 0); }
void audio_rotate()        { tone(1100, 40, 0); }
void audio_soft_drop()     { tone(700,  20, 0); }
void audio_hard_drop()     { tone(1500, 60, 0); }
void audio_lock()          { tone(500,  50, 0); }
void audio_level_up()      { tone(1600,150, 0); }
void audio_game_over_sfx() { tone(200, 400, 0); }

void audio_line_clear(int n)
{
#ifdef USE_I2S_AUDIO
    if (n <= 0 || g_volume <= 0) return;

    const int sample_rate = GB_AUDIO_SAMPLE_RATE;
    int ms = (n == 1 ? 80 : n == 2 ? 120 : n == 3 ? 160 : 220);
    int total_samples = (sample_rate * ms) / 1000;
    if (total_samples <= 0) return;

    int amp_sq = (g_volume * 2500) / 8;
    int amp_ns = (g_volume * 1500) / 8;

    int16_t buf_sq[GB_AUDIO_BUFFER_SAMPLE_COUNT];
    int16_t buf_ns[GB_AUDIO_BUFFER_SAMPLE_COUNT];
    int16_t mix   [GB_AUDIO_BUFFER_SAMPLE_COUNT];

    int freq = (n == 1 ? 600 : n == 2 ? 800 : n == 3 ? 1000 : 1300);

    while (total_samples > 0) {
        int chunk = total_samples;
        if (chunk > GB_AUDIO_BUFFER_SAMPLE_COUNT)
            chunk = GB_AUDIO_BUFFER_SAMPLE_COUNT;

        generate_square(buf_sq, chunk, freq, sample_rate, amp_sq);
        generate_noise (buf_ns, chunk, amp_ns);

        for (int i = 0; i < chunk; ++i) {
            int s = (int)buf_sq[i] + (int)buf_ns[i];
            if (s > 32767) s = 32767;
            if (s < -32768) s = -32768;
            mix[i] = (int16_t)s;
        }

        gb_ll_audio_push_buffer(mix);
        total_samples -= chunk;
    }
#else
    (void)n;
#endif
}
