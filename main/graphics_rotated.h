/*
===============================================================================
  graphics_rotated.h — Primitives graphiques avec rotation d’écran (Tetris AKA)
-------------------------------------------------------------------------------
  Rôle :
    - Fournir des versions "logiques" des primitives graphiques :
        * pixel, rectangle, texte, blit
    - Appliquer une rotation 90° (portrait) en fonction de g_rotate_screen
    - Permettre un layout indépendant de l’orientation (coordonnées logiques)
    - Dessiner le texte via la police 8×8 (font8x8_basic), pixel par pixel,
      réellement roté grâce à gfx_putpixel_rot()
===============================================================================
*/

#pragma once

#include "graphics.h"     // gfx_putpixel, gfx_blitRegion, SCREEN_W, SCREEN_H
#include "config.h"       // g_rotate_screen
#include <cstring>        // strlen()

// -----------------------------------------------------------------------------
// Police 8×8 ASCII (définie dans graphics.cpp)
// -----------------------------------------------------------------------------
extern char font8x8_basic[128][8];

// -----------------------------------------------------------------------------
// Transformation des coordonnées logiques → coordonnées écran rotées
// -----------------------------------------------------------------------------
// Espace physique : 320×240 (SCREEN_W × SCREEN_H)
// - Mode horizontal : x ∈ [0..319], y ∈ [0..239]
// - Mode vertical   : on considère un espace logique 240×320, et on applique
//   une rotation 90° horaire autour de (0,0).
// -----------------------------------------------------------------------------
inline void transform_xy(int& x, int& y)
{
    if (!g_rotate_screen)
        return;

    int old_x = x;

    x = SCREEN_W - 1 - y;  // 320 - 1 - y
    y = old_x;
}


// -----------------------------------------------------------------------------
// Pixel logique (roté si nécessaire)
// -----------------------------------------------------------------------------
inline void gfx_putpixel_rot(int x, int y, uint16_t c)
{
    transform_xy(x, y);
    gfx_putpixel(x, y, c);
}

// -----------------------------------------------------------------------------
// Rectangle plein logique
// -----------------------------------------------------------------------------
inline void gfx_fillRect_rot(int x, int y, int w, int h, uint16_t c)
{
    if (!g_rotate_screen)
    {
        gfx_fillRect(x, y, w, h, c);
        return;
    }

    for (int iy = 0; iy < h; ++iy)
    for (int ix = 0; ix < w; ++ix)
    {
        int px = x + ix;
        int py = y + iy;
        transform_xy(px, py);
        gfx_putpixel(px, py, c);
    }
}

// -----------------------------------------------------------------------------
// Rectangle contour logique
// -----------------------------------------------------------------------------
inline void gfx_drawRect_rot(int x, int y, int w, int h, uint16_t c)
{
    gfx_fillRect_rot(x, y, w, 1, c);
    gfx_fillRect_rot(x, y + h - 1, w, 1, c);
    gfx_fillRect_rot(x, y, 1, h, c);
    gfx_fillRect_rot(x + w - 1, y, 1, h, c);
}

// -----------------------------------------------------------------------------
// Dessin d’un caractère 8×8 via la police bitmap (réellement roté)
// -----------------------------------------------------------------------------
inline void draw_char_rot(int x, int y, char ch, uint16_t c)
{
    unsigned char idx = static_cast<unsigned char>(ch);
    const char* glyph = font8x8_basic[idx];

    for (int gy = 0; gy < 8; ++gy)
    {
        char row = glyph[gy];

        for (int gx = 0; gx < 8; ++gx)
        {
            if (row & (1 << gx))
            {
                int px = x + gx;
                int py = y + gy;

                gfx_putpixel_rot(px, py, c);
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Texte horizontal logique (roté si nécessaire)
// -----------------------------------------------------------------------------
inline void gfx_text_rot(int x, int y, const char* s, uint16_t c)
{
    for (int i = 0; s[i]; ++i)
    {
        draw_char_rot(x, y, s[i], c);
        x += 8; // avance horizontale (8 px)
    }
}

// -----------------------------------------------------------------------------
// Texte centré horizontalement dans l’espace logique
// -----------------------------------------------------------------------------
inline void gfx_text_center_rot(int y, const char* s, uint16_t c)
{
    int text_w = static_cast<int>(std::strlen(s)) * 8;
    int x = (SCREEN_W - text_w) / 2;

    for (int i = 0; s[i]; ++i)
    {
        draw_char_rot(x, y, s[i], c);
        x += 8;
    }
}

// -----------------------------------------------------------------------------
// Texte vertical logique (pour le mode portrait, HUD, etc.)
// Chaque lettre est dessinée sous la précédente dans l’espace logique
// -----------------------------------------------------------------------------
inline void gfx_text_vertical_rot(int x, int y, const char* s, uint16_t c)
{
    for (int i = 0; s[i]; ++i)
    {
        draw_char_rot(x, y, s[i], c);
        y += 9; // 8 px + 1 px d’espacement
    }
}

// -----------------------------------------------------------------------------
// Blit d’une région (sprites, flags, pièces) dans l’espace logique
// -----------------------------------------------------------------------------
inline void gfx_blitRegion_rot(
    const uint16_t* atlas,
    int atlas_w,
    int sx, int sy,
    int w, int h,
    int dx, int dy)
{
    if (!g_rotate_screen)
    {
        gfx_blitRegion(atlas, atlas_w, sx, sy, w, h, dx, dy);
        return;
    }

    for (int iy = 0; iy < h; ++iy)
    for (int ix = 0; ix < w; ++ix)
    {
        uint16_t col = atlas[(sy + iy) * atlas_w + (sx + ix)];

        int px = dx + ix;
        int py = dy + iy;

        transform_xy(px, py);
        gfx_putpixel(px, py, col);
    }
}
