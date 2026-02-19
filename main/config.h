/*
===============================================================================
  config.h — Configuration globale du moteur Tetris AKA
-------------------------------------------------------------------------------
  Rôle :
    - Définir les constantes globales du moteur (écran, grille, alignements).
    - Centraliser les paramètres communs (résolution, debug, framebuffer).
    - Déclarer les variables de configuration persistante :
        * g_rotate_screen
        * volume_music
        * volume_sfx
    - Fournir l’API de persistance :
        * load_config()
        * save_config()
===============================================================================
*/

#pragma once

#include <cstdint>


// ============================================================================
//  Framebuffer (optionnel)
// ============================================================================
#ifndef USE_FRAMEBUFFER
#define USE_FRAMEBUFFER 1
#endif

// ============================================================================
//  Dimensions de l’écran (mode paysage par défaut)
// ============================================================================
constexpr int SCREEN_W = 320;
constexpr int SCREEN_H = 240;

// Orientation écran (false = paysage, true = portrait)
extern bool g_rotate_screen;

// ============================================================================
//  Dimensions de la grille (génériques, pas celles du Tetris)
// ============================================================================
constexpr int TILE_SIZE = 16;                        // taille d’une tuile
constexpr int GRID_COLS = SCREEN_W / TILE_SIZE;      // colonnes visibles
constexpr int GRID_ROWS = SCREEN_H / TILE_SIZE;      // lignes visibles

// Origine de la grille (pixels)
constexpr int GRID_X0 = 0;
constexpr int GRID_Y0 = 0;

// ============================================================================
//  Tolérances d’alignement
// ============================================================================
constexpr int CENTER_EPS = 2;   // tolérance pour centrage
constexpr int SNAP_EPS   = 3;   // seuil de "snap" automatique

// ============================================================================
//  Mode debug (0 = off, 1 = on)
// ============================================================================
extern int debug;

// ============================================================================
//  Configuration persistante (stockée via NVS)
// ============================================================================
extern uint8_t volume_music;   // 0–10
extern uint8_t volume_sfx;     // 0–10

void load_config();
void save_config();
