/*
===============================================================================
  input_rotated.cpp — Remapping des touches selon l’orientation (Tetris AKA)
-------------------------------------------------------------------------------
  Rôle :
    - Appliquer une rotation 90° au D‑Pad lorsque g_rotate_screen = true.
    - Les boutons A, B, MENU ne sont jamais remappés.
===============================================================================
*/

#include "input_rotated.h"

// -----------------------------------------------------------------------------
// Remapping du D‑Pad
// -----------------------------------------------------------------------------
int remap_button(int btn)
{
    if (!g_rotate_screen)
        return btn;

    switch (btn)
    {
        case BTN_UP:    return BTN_RIGHT;
        case BTN_RIGHT: return BTN_DOWN;
        case BTN_DOWN:  return BTN_LEFT;
        case BTN_LEFT:  return BTN_UP;

        default:
            return btn; // A, B, MENU, etc.
    }
}

// -----------------------------------------------------------------------------
// Fonctions rotées
// -----------------------------------------------------------------------------
bool button_pressed_rot(int btn)
{
    return button_pressed(remap_button(btn));
}

bool button_down_rot(int btn)
{
    return button_down(remap_button(btn));
}

