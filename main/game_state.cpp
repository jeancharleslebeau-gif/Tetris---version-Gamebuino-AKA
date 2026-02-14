/*
===============================================================================
  game_state.cpp — État interne du Tetris (AKA Edition)
===============================================================================
*/

#include "game_state.h"
#include <cstring>
#include "esp_random.h"

static GameState g_state;

GameState& game_state()
{
    return g_state;
}

void game_state_reset()
{
    // Reset de la grille
    memset(g_state.field, 0, sizeof(g_state.field));

    // Score / progression
    g_state.currentLevel = 0;       // comme ton ancien code
    g_state.score = 0;
    g_state.linesCleared = 0;

    // Pièces aléatoires
    g_state.currentPiece = esp_random() % 7;
    g_state.nextPiece    = esp_random() % 7;

    // Position initiale (centre)
    g_state.currentRot = 0;
    g_state.currentX = TETRIS_COLS / 2 - 2;   // = 3 pour 10 colonnes
    g_state.currentY = 0;

    // Flags
    g_state.hasWon = false;
    g_state.hasDied = false;
}
