#pragma once

#include <stdint.h>

// -----------------------------------------------------------------------------
// Identifiants de musiques logiques pour le jeu
// -----------------------------------------------------------------------------
enum class MusicID : uint8_t
{
    MENU,       // écran titre
    GAME,       // musique in‑game (choisie selon le niveau)
    GAMEOVER,   // écran Game Over
    HISCORE,    // écran High Score / pause
    NONE
};

// -----------------------------------------------------------------------------
// API simple utilisée par game.cpp
// -----------------------------------------------------------------------------

// À appeler une fois au démarrage du jeu
void audio_pmf_init();

// À appeler à chaque frame (depuis ta boucle principale ou ton système audio)
void audio_pmf_update();

// Joue une musique logique (MENU, GAME, GAMEOVER, HISCORE)
void audio_pmf_play(MusicID id, bool loop);

// Coupe la musique PMF en cours
void audio_pmf_stop();

// Informe le module audio du niveau courant pour choisir la bonne musique in‑game
void audio_pmf_set_level(uint8_t level);

// Réglage du volume musique PMF (0..255) 
void audio_pmf_set_master_volume(uint8_t v);




