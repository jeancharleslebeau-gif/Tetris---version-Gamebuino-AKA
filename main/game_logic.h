/*
===============================================================================
  game_logic.h — Logique interne du Tetris (AKA Edition)
-------------------------------------------------------------------------------
  API exposée à game.cpp :
    - logic_reset()
    - game_tick()
    - game_move_left()
    - game_move_right()
    - game_rotate()
    - game_soft_drop()
    - game_hard_drop()
===============================================================================
*/

#pragma once

// Reset complet du jeu
void logic_reset();

// Tick logique (gravité + lock + clear lines)
void game_tick();

// Actions déclenchées par game.cpp (inputs rotés)
void game_move_left();
void game_move_right();
void game_rotate();
void game_rotate_cw(); 
void game_rotate_ccw();
void game_soft_drop();
void game_hard_drop();
