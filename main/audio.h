#pragma once
/*
===============================================================================
  audio.h — Petit wrapper audio pour Tetris AKA
-------------------------------------------------------------------------------
  Rôle :
    - Initialiser le PWM (LEDC) pour le buzzer.
    - Fournir des fonctions simples : tone(), click(), audio_line_clear(), etc.
    - Gestion d’un volume global (0..8).
===============================================================================
*/

#ifdef __cplusplus
extern "C" {
#endif

// Initialise le système audio (LEDC, GPIO, etc.)
void audio_init();

// Réglage du volume global (0 = mute, 8 = max)
void audio_set_volume(int v);

// Joue un tone générique
//  - freq : fréquence en Hz
//  - ms   : durée en millisecondes (si 0 => tone continu jusqu’à arrêt)
//  - duty : duty cycle brut (si 0 => calculé à partir du volume)
void tone(int freq, int ms = 0, int duty = 0);

// Petits sons utilitaires
void click();
void audio_hard_drop();

// Son de clear de lignes (1..4)
void audio_line_clear(int n);

#ifdef __cplusplus
}
#endif
