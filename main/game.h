/*
===============================================================================
  game.h — Interface principale du jeu Tetris (AKA Edition)
-------------------------------------------------------------------------------
  Rôle :
    - Définir les états du jeu.
    - Fournir les fonctions appelées par task_game().
===============================================================================
*/

#pragma once
#include <stdint.h>
#include "game_state.h"

extern int pause_selection;

// -----------------------------------------------------------------------------
//  États du jeu
// -----------------------------------------------------------------------------
enum class GameMode
{
    Title,
    Playing,
    Paused,
    GameOver
};

// -----------------------------------------------------------------------------
//  Accès à l’état courant
// -----------------------------------------------------------------------------
GameMode& game_mode();

// -----------------------------------------------------------------------------
//  API du jeu
// -----------------------------------------------------------------------------
void game_init();          // Initialisation du jeu
void game_update();        // Logique (appelée à chaque frame)
void game_render();        // Rendu (appelé à chaque frame)