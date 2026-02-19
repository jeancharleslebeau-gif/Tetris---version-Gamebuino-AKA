/*
===============================================================================
  input_rotated.h — Remapping des touches selon l’orientation (Tetris AKA)
-------------------------------------------------------------------------------
  Rôle :
    - Fournir des versions rotées des fonctions input :
        * button_pressed_rot()
        * button_down_rot()

    - En mode portrait (g_rotate_screen = true), le D‑Pad est tourné de 90° :
        UP    → RIGHT
        RIGHT → DOWN
        DOWN  → LEFT
        LEFT  → UP
    - Les boutons A, B, MENU restent inchangés.
===============================================================================
*/

#pragma once

#include "input.h"
#include "config.h"

// Remap d’un bouton selon l’orientation
int remap_button(int btn);

// Fonctions rotées
bool button_pressed_rot(int btn);
bool button_down_rot(int btn);

