/*
===============================================================================
  render.h — Rendu graphique du Tetris (AKA Edition)
-------------------------------------------------------------------------------
  Rôle :
    - Dessiner la grille (blocs posés)
    - Dessiner la pièce courante
    - Dessiner la prochaine pièce (preview)
    - Afficher score, lignes, niveau
    - Utiliser l’API graphique générique (core/graphics.h)
===============================================================================
*/

#pragma once

// Rendu complet du jeu (appelé depuis game_render)
void render_game();

// Rendu de l’écran titre
void render_title();

// Rendu de l’écran Game Over
void render_game_over();

// Rendu de l'écran de pause / menu
void render_pause_menu();
