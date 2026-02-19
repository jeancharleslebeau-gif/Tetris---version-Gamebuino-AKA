/*
===============================================================================
  audio.cpp — Système audio Tetris AKA
===============================================================================
*/

#define USE_I2S_AUDIO

#include "audio.h"
#include "gb_ll_audio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "gb_audio_player.h"
#include "gb_audio_track_tone.h"
#include "gb_audio_track_pmf.h"
#include "gb_audio_track_wav.h"

#include "audio_pmf.h"
#include "config.h"   

// -----------------------------------------------------------------------------
// Tâche PMF
// -----------------------------------------------------------------------------
static TaskHandle_t s_pmf_task = nullptr;

static void pmf_task(void* arg)
{
    (void)arg;
    while (true)
    {
        audio_pmf_update();           // → g_player.pool()
        vTaskDelay(pdMS_TO_TICKS(2)); // toutes les ~2 ms
    }
}

// -----------------------------------------------------------------------------
// Player & pistes (définition globale)
// -----------------------------------------------------------------------------
gb_audio_player     g_player;
gb_audio_track_pmf  g_track_music;
gb_audio_track_tone g_track_tone;
gb_audio_track_wav  g_track_wav;

// -----------------------------------------------------------------------------
// Volumes globaux (définis dans config.cpp)
// -----------------------------------------------------------------------------
extern uint8_t volume_music;   // 0..10
extern uint8_t volume_sfx;     // 0..10

static bool sfx_feedback_enabled = false;

// -----------------------------------------------------------------------------
// Mapping volumes
// -----------------------------------------------------------------------------
static inline uint8_t map_music_volume_to_hw(int v)
{
    if (v < 0) v = 0;
    if (v > 10) v = 10;
    return (uint8_t)(v * 25);   // 0..250
}

static inline float map_sfx_volume_to_float(int v)
{
    if (v < 0) v = 0;
    if (v > 10) v = 10;
    return v / 10.0f;
}

// -----------------------------------------------------------------------------
// Volume SFX silencieux (init)
// -----------------------------------------------------------------------------
void audio_set_sfx_volume_silent(int v)
{
    if (v < 0) v = 0;
    if (v > 10) v = 10;
    volume_sfx = v;
}

// -----------------------------------------------------------------------------
// Active le bip après init
// -----------------------------------------------------------------------------
static void audio_enable_sfx_feedback()
{
    sfx_feedback_enabled = true;
}

// -----------------------------------------------------------------------------
// INIT
// -----------------------------------------------------------------------------
void audio_init()
{
#ifdef USE_I2S_AUDIO
    gb_ll_audio_set_volume(map_music_volume_to_hw(volume_music));
#endif

    // Initialise PMF
    audio_pmf_init();

    // Ajout des pistes
    g_player.add_track(&g_track_music);
    g_player.add_track(&g_track_tone);
    g_player.add_track(&g_track_wav);

    // Volume interne des pistes
    g_track_tone.set_track_volume(0.8f);
    g_track_wav.set_track_volume(0.8f);

    // Tâche PMF
    if (s_pmf_task == nullptr)
    {
        xTaskCreatePinnedToCore(
            pmf_task,
            "pmf_audio",
            4096,
            nullptr,
            4,
            &s_pmf_task,
            1
        );
    }

    audio_enable_sfx_feedback();
}

// -----------------------------------------------------------------------------
// Volume musique
// -----------------------------------------------------------------------------
void audio_set_music_volume(int v)
{
    if (v < 0) v = 0;
    if (v > 10) v = 10;

    volume_music = v;

#ifdef USE_I2S_AUDIO
    gb_ll_audio_set_volume(map_music_volume_to_hw(volume_music));
#endif
}

// -----------------------------------------------------------------------------
// Volume SFX
// -----------------------------------------------------------------------------
void audio_set_sfx_volume(int v)
{
    if (v < 0) v = 0;
    if (v > 10) v = 10;

    volume_sfx = v;

    if (sfx_feedback_enabled)
        tone(1000, 40, 0);
}

// -----------------------------------------------------------------------------
// Tone SFX
// -----------------------------------------------------------------------------
void tone(int freq, int ms, int duty)
{
    (void)duty;

    if (volume_sfx <= 0 || ms <= 0)
        return;

    float vol = map_sfx_volume_to_float(volume_sfx);

    g_track_tone.play_tone(
        freq,
        vol,
        ms,
        gb_audio_track_tone::SINE
    );
}

// -----------------------------------------------------------------------------
// Musiques PMF
// -----------------------------------------------------------------------------
void audio_play_music(MusicType type)
{
    switch (type)
    {
        case MUSIC_MENU:     audio_pmf_play(MusicID::MENU, true); break;
        case MUSIC_GAME:     audio_pmf_play(MusicID::GAME, true); break;
        case MUSIC_GAMEOVER: audio_pmf_play(MusicID::GAMEOVER, false); break;
        default:             audio_pmf_stop(); break;
    }
}

void audio_stop_music()
{
    audio_pmf_stop();
}

// -----------------------------------------------------------------------------
// SFX Tetris
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
    if (n <= 0 || volume_sfx <= 0) return;

    float vol = map_sfx_volume_to_float(volume_sfx);

    int freq = (n == 1 ? 600 : n == 2 ? 800 : n == 3 ? 1000 : 1300);
    int ms   = (n == 1 ? 80  : n == 2 ? 120 : n == 3 ? 160  : 220);

    g_track_tone.play_tone(
        freq,
        vol,
        ms,
        gb_audio_track_tone::SQUARE
    );
}
