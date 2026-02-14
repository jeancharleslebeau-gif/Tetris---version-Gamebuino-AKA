/*
===============================================================================
  graphics.h — API graphique du moteur AKA
-------------------------------------------------------------------------------
  Rôle :
    - Fournir une couche graphique portable et indépendante du matériel.
    - Encapsuler les primitives bas niveau Gamebuino (LCD, DMA, backlight).
    - Offrir des fonctions simples pour :
        * pixels
        * lignes / rectangles / cercles
        * blit d’images RGB565
        * texte 8×8
    - Ne contient AUCUNE logique gameplay.
===============================================================================
*/

#pragma once
#include <stdint.h>

// ============================================================================
//  Type Color — Couleur 16 bits (format RGB565)
// ============================================================================
using Color = uint16_t;


// ============================================================================
//  Initialisation du système graphique
// ----------------------------------------------------------------------------
//  - Configure gb_graphics (backlight, pipeline LCD)
//  - Doit être appelé après gb_core.init()
// ============================================================================
void gfx_init();


// ============================================================================
//  Effacement / Présentation
// ============================================================================
void gfx_clear(Color color);     // Efface l’écran
void gfx_present();              // Rafraîchit le LCD
void gfx_flush();                // Alias pratique


// ============================================================================
//  Pixel et primitives de base
// ============================================================================
void gfx_putpixel(int x, int y, Color color);

// Rectangle plein
void gfx_fillRect(int x, int y, int w, int h, Color color);

// Rectangle vide (contour)
void gfx_drawRect(int x, int y, int w, int h, Color color);

// Ligne (algorithme de Bresenham)
void gfx_drawLine(int x0, int y0, int x1, int y1, Color color);

// Cercle vide (Midpoint circle)
void gfx_drawCircle(int cx, int cy, int r, Color color);

// Cercle plein
void gfx_fillCircle(int cx, int cy, int r, Color color);


// ============================================================================
//  Blit d’un bitmap arbitraire (ex: 16×16, 320×240…)
// ============================================================================
void gfx_blit(const uint16_t* pixels, int w, int h, int x, int y);


// ============================================================================
//  Blit d’une région rectangulaire dans un atlas 16 bits (RGB565)
// ----------------------------------------------------------------------------
//  - atlas = image brute (ex: 256×32) rangée ligne par ligne
//  - srcX, srcY, srcW, srcH = rectangle source dans l’atlas
//  - dstX, dstY = position de destination à l’écran
// ============================================================================
void gfx_blitRegion(
    const uint16_t* atlas,
    int atlasW,
    int srcX, int srcY,
    int srcW, int srcH,
    int dstX, int dstY
);


// ============================================================================
//  Blit d’une région rectangulaire avec scaling (Q8.8)
// ----------------------------------------------------------------------------
//  - scale_fp = facteur de zoom en Q8.8 (256 = 1.0).
// ============================================================================
void gfx_blitRegionScaled(
    const uint16_t* atlas,
    int atlasW,
    int srcX, int srcY,
    int srcW, int srcH,
    int dstX, int dstY,
    int scale_fp
);


// ============================================================================
//  Blit d’un bitmap arbitraire avec scaling (Q8.8)
// ============================================================================
void gfx_blitScaled(
    const uint16_t* pixels,
    int srcW, int srcH,
    int dstX, int dstY,
    int scale_fp
);


// ============================================================================
//  Texte (font8x8)
// ============================================================================
void gfx_text(int x, int y, const char* text, Color color);
void gfx_text_center(int y, const char* text, Color color);


// ============================================================================
//  Dimensions écran
// ============================================================================
int gfx_width();
int gfx_height();


// ============================================================================
// Palette de couleurs standard (BGR565)
// ============================================================================
static constexpr Color COLOR_BLACK      = 0x0000;
static constexpr Color COLOR_WHITE      = 0xFFFF;
static constexpr Color COLOR_RED        = 0x001F;  
static constexpr Color COLOR_GREEN      = 0x03E0;   
static constexpr Color COLOR_BLUE       = 0xF800;   
static constexpr Color COLOR_GRAY       = 0x84D5;
static constexpr Color COLOR_DARKGRAY   = 0x426A;
static constexpr Color COLOR_PURPLE     = 0x4012;
static constexpr Color COLOR_PINK       = 0x8239;
static constexpr Color COLOR_ORANGE     = 0x155F;
static constexpr Color COLOR_BROWN      = 0x4479;
static constexpr Color COLOR_BEIGE      = 0x96BF;
static constexpr Color COLOR_YELLOW     = 0x073E;   
static constexpr Color COLOR_LIGHTGREEN = 0x07E0;   
static constexpr Color COLOR_DARKBLUE   = 0x8200;
static constexpr Color COLOR_LIGHTBLUE  = 0xFDCF;
static constexpr Color COLOR_SILVER     = 0xBDF7;
static constexpr Color COLOR_GOLD       = 0x159C;