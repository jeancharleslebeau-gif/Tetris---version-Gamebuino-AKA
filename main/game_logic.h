/*
===============================================================================
  game_logic.h — Logique interne du Tetris (AKA Edition)
-------------------------------------------------------------------------------
  Fonctions exposées à game.cpp :
    - logic_reset()
    - game_update_logic()
    - game_render_logic()
===============================================================================
*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Reset complet du jeu
void logic_reset();

// Update logique (appelé à chaque frame)
void game_update_logic();

// Rendu logique (appelé par render_game)
void game_render_logic();


#ifdef __cplusplus
}
#endif
