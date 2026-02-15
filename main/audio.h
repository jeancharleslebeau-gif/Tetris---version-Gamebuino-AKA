/*
===============================================================================
  audio.h — Système audio haut niveau pour Tetris AKA
-------------------------------------------------------------------------------
  - SFX Tetris (move, rotate, drop, clear, level up, game over)
  - Musique Tetris style Game Boy (square wave) via tâche FreeRTOS
  - Utilise la couche I2S + TAS2505 de Gamebuino AKA (gb_ll_audio)
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

// Initialisation
void audio_init();
void audio_set_volume(int v);

// SFX
void click();
void audio_move();
void audio_rotate();
void audio_soft_drop();
void audio_hard_drop();
void audio_lock();
void audio_level_up();
void audio_game_over_sfx();
void audio_line_clear(int n);

// Musiques
void audio_play_music(MusicType type);
void audio_stop_music();

// Tone bas niveau (utilisé par la musique)
void tone(int freq, int ms, int duty);
