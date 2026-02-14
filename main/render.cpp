/*
===============================================================================
  render.cpp — Rendu graphique du Tetris (AKA Edition)
-------------------------------------------------------------------------------

    - Dessine la grille, la pièce courante et la prochaine pièce
    - Affiche score, lignes, niveau
    - Utilise gfx_fillRect(), gfx_text(), gfx_drawRect(), gfx_flush()
    - Utilise pieces.h comme source unique des pièces
===============================================================================
*/

#include "render.h"
#include "graphics.h"
#include "game_state.h"
#include "pieces.h"     // ⭐ Source unique des pièces
#include <string>

// -----------------------------------------------------------------------------
//  Paramètres d’affichage
// -----------------------------------------------------------------------------
static constexpr int BLOCK  = 10;
static constexpr int GRID_X = 40;
static constexpr int GRID_Y = 20;

static constexpr int WELL_W = TETRIS_COLS * BLOCK;
static constexpr int WELL_H = TETRIS_ROWS * BLOCK;

// -----------------------------------------------------------------------------
//  Couleurs des blocs (index 0 = vide)
// -----------------------------------------------------------------------------
static const Color COLORS[8] = {
    COLOR_BLACK,     // 0 = vide
    COLOR_RED,       // 1
    COLOR_GREEN,     // 2
    COLOR_BLUE,      // 3
    COLOR_YELLOW,    // 4
    COLOR_PURPLE,    // 5
    COLOR_LIGHTBLUE, // 6
    COLOR_WHITE      // 7
};

// -----------------------------------------------------------------------------
//  Dessine un bloc coloré (en pixels)
// -----------------------------------------------------------------------------
static void draw_block(int x, int y, int color)
{
    gfx_fillRect(x, y, BLOCK, BLOCK, COLORS[color]);
}

// -----------------------------------------------------------------------------
//  Cadre du puits
// -----------------------------------------------------------------------------
static void render_well_frame()
{
    gfx_drawRect(GRID_X - 4, GRID_Y - 4, WELL_W + 8, WELL_H + 8, COLOR_WHITE);
    gfx_drawRect(GRID_X - 2, GRID_Y - 2, WELL_W + 4, WELL_H + 4, COLOR_GRAY);
    gfx_fillRect(GRID_X, GRID_Y, WELL_W, WELL_H, COLOR_DARKGRAY);
}

// -----------------------------------------------------------------------------
//  Dessine la grille (blocs posés)
// -----------------------------------------------------------------------------
static void render_grid()
{
    auto& S = game_state();

    for(int y = 0; y < TETRIS_ROWS; ++y)
    {
        for(int x = 0; x < TETRIS_COLS; ++x)
        {
            int v = S.field[y][x];
            if(v != 0)
            {
                int sx = GRID_X + x * BLOCK;
                int sy = GRID_Y + y * BLOCK;
                draw_block(sx, sy, v);
            }
        }
    }
}

// -----------------------------------------------------------------------------
//  Dessine la pièce courante
// -----------------------------------------------------------------------------
static void render_piece()
{
    auto& S = game_state();

    for(int py = 0; py < 4; ++py)
    {
        for(int px = 0; px < 4; ++px)
        {
            if(!piece_at(S.currentPiece, S.currentRot, py, px))
                continue;

            int gx = S.currentX + px;
            int gy = S.currentY + py;

            // ⭐ Ne pas dessiner les cases au-dessus de la grille
            if(gy < 0 || gy >= TETRIS_ROWS)
                continue;
            if(gx < 0 || gx >= TETRIS_COLS)
                continue;

            int sx = GRID_X + gx * BLOCK;
            int sy = GRID_Y + gy * BLOCK;
            draw_block(sx, sy, S.currentPiece + 1);
        }
    }
}

// -----------------------------------------------------------------------------
//  HUD (score, lignes, niveau, titre, label Next)
// -----------------------------------------------------------------------------
static void render_hud()
{
    auto& S = game_state();

    int x = GRID_X + WELL_W + 20;
    int y = 20;

    gfx_text(x, y, "TETRIS", COLOR_YELLOW);
    y += 20;

    gfx_text(x, y, "Score:", COLOR_WHITE);
    gfx_text(x, y + 12, std::to_string(S.score).c_str(), COLOR_LIGHTBLUE);
    y += 40;

    gfx_text(x, y, "Lines:", COLOR_WHITE);
    gfx_text(x, y + 12, std::to_string(S.linesCleared).c_str(), COLOR_GREEN);
    y += 40;

    gfx_text(x, y, "Level:", COLOR_WHITE);
    gfx_text(x, y + 12, std::to_string(S.currentLevel).c_str(), COLOR_PINK);
    y += 40;

    gfx_text(x, y, "Next:", COLOR_WHITE);
}

// -----------------------------------------------------------------------------
//  Dessine la prochaine pièce (preview)
// -----------------------------------------------------------------------------
static void render_next_piece()
{
    auto& S = game_state();

    int p = S.nextPiece;
    int baseX = GRID_X + WELL_W + 20;
    int baseY = 160 + BLOCK;

    for(int py = 0; py < 4; ++py)
    {
        for(int px = 0; px < 4; ++px)
        {
            if(piece_at(p, 0, py, px))
            {
                int sx = baseX + px * BLOCK;
                int sy = baseY + py * BLOCK;
                draw_block(sx, sy, p + 1);
            }
        }
    }
}

// -----------------------------------------------------------------------------
//  Rendu complet du jeu
// -----------------------------------------------------------------------------
void render_game()
{
    gfx_clear(COLOR_BLACK);

    render_well_frame();
    render_grid();
    render_piece();
    render_hud();
    render_next_piece();

    gfx_flush();
}

// -----------------------------------------------------------------------------
//  Écran titre
// -----------------------------------------------------------------------------
void render_title()
{
    gfx_clear(COLOR_BLACK);
    gfx_text_center(80, "TETRIS AKA", COLOR_YELLOW);
    gfx_text_center(140, "Press A to start", COLOR_WHITE);
    gfx_flush();
}

// -----------------------------------------------------------------------------
//  Écran Game Over
// -----------------------------------------------------------------------------
void render_game_over()
{
    gfx_text_center(120, "GAME OVER", COLOR_RED);
    gfx_flush();
}
