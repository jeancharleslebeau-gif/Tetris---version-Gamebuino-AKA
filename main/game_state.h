/*
===============================================================================
  game_state.h — État interne du Tetris (AKA Edition)
-------------------------------------------------------------------------------
  Contient :
    - La grille (20x10)
    - La pièce courante
    - La prochaine pièce
    - Le score, le niveau, les lignes
    - Les flags hasWon / hasDied
===============================================================================
*/

#pragma once
#include <stdint.h>

static const int TETRIS_COLS = 10;
static const int TETRIS_ROWS = 20;

struct GameState
{
    int field[TETRIS_ROWS][TETRIS_COLS] = {};

    int currentLevel = 1;
    int score = 0;
    int linesCleared = 0;

    int currentPiece = 0;
    int currentRot = 0;
    int currentX = 3;
    int currentY = 0;

    int nextPiece = 0;

    bool hasWon = false;
    bool hasDied = false;
};

GameState& game_state();
void game_state_reset();
