/*
===============================================================================
  audio.h — Système audio Tetris AKA
-------------------------------------------------------------------------------
  - Musique PMF (track_pmf)
  - SFX Tone (track_tone)
  - SFX WAV (track_wav)
  - Volumes séparés musique / SFX (0..10)
===============================================================================
*/
#pragma once
#include <stdint.h>

// -----------------------------------------------------------------------------
// Types de musiques
// -----------------------------------------------------------------------------
enum MusicType {
    MUSIC_NONE,
    MUSIC_MENU,
    MUSIC_GAME,
    MUSIC_GAMEOVER
};

// -----------------------------------------------------------------------------
// API audio publique
// -----------------------------------------------------------------------------

// Initialisation générale
void audio_init();

// Réglage des volumes (0..10)
void audio_set_music_volume(int v);
void audio_set_sfx_volume(int v);

// Version silencieuse (sans bip) pour l'initialisation
void audio_set_sfx_volume_silent(int v);

// Tone bas niveau (utilisé par les SFX)
void tone(int freq, int ms, int duty);

// Musiques PMF
void audio_play_music(MusicType type);
void audio_stop_music();

// SFX Tetris
void click();
void audio_move();
void audio_rotate();
void audio_soft_drop();
void audio_hard_drop();
void audio_lock();
void audio_level_up();
void audio_game_over_sfx();
void audio_line_clear(int n);
