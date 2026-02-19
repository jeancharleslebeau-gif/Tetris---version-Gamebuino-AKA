/*
===============================================================================
  render.cpp — Rendu graphique du Tetris (AKA Edition)
-------------------------------------------------------------------------------
  Rôle :
    - Gérer tout le rendu du jeu :
        * Écran titre (horizontal / vertical)
        * Menu Pause (centré, roté)
        * Game Over
        * Puits, grille, pièce courante
        * HUD + Next Piece
    - S’appuyer sur graphics_rotated.h pour gérer la rotation 90°
    - Adapter dynamiquement les marges et tailles selon l’orientation
===============================================================================
*/

#include "render.h"
#include "graphics.h"
#include "graphics_rotated.h"
#include "game_state.h"
#include "pieces.h"
#include "title_screen.h"
#include "title_screen_vertical.h"
#include "config.h"
#include "flags.h"
#include "language.h"
#include <string>
#include <stdint.h>

/*==============================================================================
    DIMENSIONS LOGIQUES (selon orientation)
==============================================================================*/

static inline int logical_width()
{
    return g_rotate_screen ? 240 : 320;
}

static inline int logical_height()
{
    return g_rotate_screen ? 320 : 240;
}

/*==============================================================================
    PARAMÈTRES DYNAMIQUES (mis à jour selon orientation)
==============================================================================*/

static int BLOCK  = 11;
static int GRID_X = 10;
static int GRID_Y = 10;
static int WELL_W = TETRIS_COLS * 11;
static int WELL_H = TETRIS_ROWS * 11;
static int HUD_X  = 10;
static int HUD_W  = 100;

/*==============================================================================
    PROTOTYPES INTERNES
==============================================================================*/

static void render_well_frame();
static void render_grid();
static void render_piece();
static void render_hud();
static void render_next_piece();
static void draw_block(int x, int y, int color);

/*==============================================================================
    MISE À JOUR DES PARAMÈTRES DE RENDU
==============================================================================*/

static void update_render_params()
{
    if (g_rotate_screen)
    {
        // MODE PORTRAIT (240×320 logique)
        BLOCK = 15;

        WELL_W = TETRIS_COLS * BLOCK;
        WELL_H = TETRIS_ROWS * BLOCK;

        GRID_X = 10;
        GRID_Y = 10;

        HUD_X = GRID_X + WELL_W + 10;
        HUD_W = 240 - HUD_X;
    }
    else
    {
        // MODE HORIZONTAL (320×240 logique)
        BLOCK = 11;

        WELL_W = TETRIS_COLS * BLOCK;
        WELL_H = TETRIS_ROWS * BLOCK;

        GRID_X = 10;
        GRID_Y = 10;

        HUD_X = GRID_X + WELL_W + 40;
        HUD_W = 320 - HUD_X;
    }
}

/*==============================================================================
    MENU PAUSE
==============================================================================*/

void render_pause_menu()
{
    update_render_params();
    gfx_clear(COLOR_BLACK);

    const int win_w = 220;
    const int win_h = 180;

    int lw = logical_width();
    int lh = logical_height();

    int win_x = (lw - win_w) / 2;
    int win_y = (lh - win_h) / 2;

    gfx_fillRect_rot(win_x, win_y, win_w, win_h, COLOR_DARKGRAY);
    gfx_drawRect_rot(win_x, win_y, win_w, win_h, COLOR_WHITE);

    int x = win_x + 16;
    int y = win_y + 24;

    gfx_text_rot(x + 60, y, tr(TextID::PAUSE_TITLE), COLOR_YELLOW);
    y += 28;

    extern uint8_t volume_music;
    extern uint8_t volume_sfx;
    extern int pause_selection;

    int cursor_y = y + pause_selection * 20;
    gfx_text_rot(x - 12, cursor_y, ">", COLOR_WHITE);

    char buf[64];

    sprintf(buf, "%s : %d", tr(TextID::PAUSE_MUSIC_LABEL), volume_music);
    gfx_text_rot(x, y , buf, COLOR_WHITE);

    sprintf(buf, "%s : %d", tr(TextID::PAUSE_SFX_LABEL), volume_sfx);
    gfx_text_rot(x, y + 20, buf, COLOR_WHITE);

    sprintf(buf, "%s :", tr(TextID::PAUSE_LANGUAGE_LABEL));
    gfx_text_rot(x, y + 40, buf, COLOR_WHITE);

    int flag_index = (int)language_get();
    int fx = x + 70;
    int fy = y + 30;

    gfx_blitRegion_rot(flags_atlas, FLAGS_ATLAS_WIDTH,
                       flag_index * FLAG_WIDTH, 0,
                       FLAG_WIDTH, FLAG_HEIGHT,
                       fx, fy);

    gfx_text_rot(fx + FLAG_WIDTH + 10, y + 40, language_name(language_get()), COLOR_WHITE);

    sprintf(buf, "%s : %s",
            tr(TextID::PAUSE_ORIENTATION_LABEL),
            g_rotate_screen ? tr(TextID::PAUSE_ORIENTATION_ON)
                            : tr(TextID::PAUSE_ORIENTATION_OFF));
    gfx_text_rot(x, y + 60, buf, COLOR_WHITE);

    gfx_text_rot(x, y + 80, tr(TextID::PAUSE_BACK_TO_GAME), COLOR_LIGHTBLUE);
    gfx_text_rot(x, y + 100, tr(TextID::PAUSE_BACK_TO_MENU), COLOR_LIGHTBLUE);

    gfx_flush();
}

/*==============================================================================
    GAME OVER
==============================================================================*/

void render_game_over()
{
    update_render_params();
    gfx_clear(COLOR_BLACK);

    const char* txt = tr(TextID::GAME_OVER);

    int lw = logical_width();
    int lh = logical_height();

    int text_w = strlen(txt) * 8;
    int x = (lw - text_w) / 2;
    int y = (lh - 8) / 2;

    gfx_text_rot(x, y, txt, COLOR_RED);

    gfx_flush();
}


/*==============================================================================
    ÉCRAN TITRE
==============================================================================*/

void render_title()
{
    update_render_params();
    gfx_clear(COLOR_BLACK);

    if (g_rotate_screen)
    {
        // Image déjà tournée à 90° dans title_screen_vertical
        for (int y = 0; y < 240; ++y)
        for (int x = 0; x < 320; ++x)
            gfx_putpixel(x, y, title_screen_vertical[y][x]);
    }
    else
    {
        for (int y = 0; y < 240; ++y)
        for (int x = 0; x < 320; ++x)
            gfx_putpixel(x, y, title_screen[y][x]);
    }

    // Drapeau + langue (rotés)
    int fx = 20;
    int fy = logical_height() - 50;

    int flag_index = (int)language_get();

    gfx_blitRegion_rot(flags_atlas, FLAGS_ATLAS_WIDTH,
                       flag_index * FLAG_WIDTH, 0,
                       FLAG_WIDTH, FLAG_HEIGHT,
                       fx, fy);

    gfx_text_rot(fx + FLAG_WIDTH + 8, fy + 4, language_name(language_get()), COLOR_WHITE);

    gfx_flush();
}

/*==============================================================================
    RENDU DU JEU
==============================================================================*/

void render_game()
{
    update_render_params();
    gfx_clear(COLOR_BLACK);

    render_well_frame();
    render_grid();
    render_piece();
    render_hud();
    render_next_piece();

    gfx_flush();
}

/*==============================================================================
    BLOCS INTERNES : PUITS, GRILLE, PIÈCE, HUD, NEXT
==============================================================================*/

static void render_well_frame()
{
    gfx_drawRect_rot(GRID_X - 2, GRID_Y - 2, WELL_W + 4, WELL_H + 4, COLOR_WHITE);
    gfx_drawRect_rot(GRID_X - 1, GRID_Y - 1, WELL_W + 2, WELL_H + 2, COLOR_GRAY);
    gfx_fillRect_rot(GRID_X, GRID_Y, WELL_W, WELL_H, COLOR_DARKGRAY);
}

static void render_grid()
{
    auto& S = game_state();

    for (int y = 0; y < TETRIS_ROWS; ++y)
    for (int x = 0; x < TETRIS_COLS; ++x)
    {
        int v = S.field[y][x];
        if (v != 0)
        {
            int sx = GRID_X + x * BLOCK;
            int sy = GRID_Y + y * BLOCK;
            draw_block(sx, sy, v);
        }
    }
}

static void render_piece()
{
    auto& S = game_state();

    for (int py = 0; py < 4; ++py)
    for (int px = 0; px < 4; ++px)
    {
        if (!piece_at(S.currentPiece, S.currentRot, py, px))
            continue;

        int gx = S.currentX + px;
        int gy = S.currentY + py;

        if (gx < 0 || gx >= TETRIS_COLS) continue;
        if (gy < 0 || gy >= TETRIS_ROWS) continue;

        int sx = GRID_X + gx * BLOCK;
        int sy = GRID_Y + gy * BLOCK;
        draw_block(sx, sy, S.currentPiece + 1);
    }
}

static void render_hud()
{
    auto& S = game_state();

    int x = HUD_X;
    int y = 16;

    gfx_text_rot(x, y, tr(TextID::HUD_TITLE_TETRIS), COLOR_YELLOW);
    y += 24;

    gfx_text_rot(x, y, tr(TextID::HUD_SCORE), COLOR_WHITE);
    gfx_text_rot(x, y + 12, std::to_string(S.score).c_str(), COLOR_LIGHTBLUE);
    y += 32;

    gfx_text_rot(x, y, tr(TextID::HUD_LINES), COLOR_WHITE);
    gfx_text_rot(x, y + 12, std::to_string(S.linesCleared).c_str(), COLOR_GREEN);
    y += 32;

    gfx_text_rot(x, y, tr(TextID::HUD_LEVEL), COLOR_WHITE);
    gfx_text_rot(x, y + 12, std::to_string(S.currentLevel).c_str(), COLOR_PINK);
    y += 32;

    gfx_text_rot(x, y, tr(TextID::HUD_NEXT), COLOR_WHITE);
}

static void render_next_piece()
{
    auto& S = game_state();
    int p = S.nextPiece;

    int baseX = HUD_X + 8;
    int baseY = 160;

    for (int py = 0; py < 4; ++py)
    for (int px = 0; px < 4; ++px)
    {
        if (piece_at(p, 0, py, px))
        {
            int sx = baseX + px * BLOCK;
            int sy = baseY + py * BLOCK;
            draw_block(sx, sy, p + 1);
        }
    }
}

/*==============================================================================
    DESSIN D’UN BLOC 3D
==============================================================================*/

static const Color COLORS[8] = {
    COLOR_BLACK,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_YELLOW,
    COLOR_PURPLE,
    COLOR_LIGHTBLUE,
    COLOR_WHITE
};

static inline int clampi(int v, int lo, int hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static uint16_t adjust_color(uint16_t c, int delta)
{
    int r = (c >> 11) & 0x1F;
    int g = (c >> 5)  & 0x3F;
    int b =  c        & 0x1F;

    r = clampi(r + delta, 0, 31);
    g = clampi(g + delta, 0, 63);
    b = clampi(b + delta, 0, 31);

    return (uint16_t)((r << 11) | (g << 5) | b);
}

static uint16_t normalize_color(uint16_t c)
{
    int r = ((c >> 11) & 0x1F) * 20 / 31;
    int g = ((c >> 5)  & 0x3F) * 32 / 63;
    int b = ( c        & 0x1F) * 20 / 31;

    return (uint16_t)((r << 11) | (g << 5) | b);
}

static void compute_shaded_colors(uint16_t base,
                                  uint16_t& light_top,
                                  uint16_t& light_left,
                                  uint16_t& shadow_bottom,
                                  uint16_t& shadow_right,
                                  uint16_t& glossy)
{
    base = normalize_color(base);

    light_top     = adjust_color(base, +18);
    light_left    = adjust_color(base, +14);
    shadow_bottom = adjust_color(base, -18);
    shadow_right  = adjust_color(base, -14);
    glossy        = adjust_color(base, +24);
}

static void draw_block_3d(int x, int y, uint16_t base)
{
    const int W = BLOCK;
    const int H = BLOCK;
    const int BEVEL = (BLOCK <= 11 ? 2 : 3);

    uint16_t lt, ll, sb, sr, glossy;
    compute_shaded_colors(base, lt, ll, sb, sr, glossy);

    gfx_fillRect_rot(x, y, W, BEVEL, lt);
    gfx_fillRect_rot(x, y, BEVEL, H, ll);
    gfx_fillRect_rot(x, y + H - BEVEL, W, BEVEL, sb);
    gfx_fillRect_rot(x + W - BEVEL, y, BEVEL, H, sr);
    gfx_fillRect_rot(x + 2, y + 2, 4, 3, glossy);

    int innerX = x + BEVEL;
    int innerY = y + BEVEL;
    int innerW = W - BEVEL * 2;
    int innerH = H - BEVEL * 2;

    for (int i = 0; i < innerH; ++i)
    {
        int dy = innerY + i;
        int delta = (i * -10) / innerH;
        uint16_t c = adjust_color(base, delta);
        gfx_fillRect_rot(innerX, dy, innerW, 1, c);
    }
}

static void draw_block(int x, int y, int color)
{
    if (color <= 0 || color >= 8)
        return;

    draw_block_3d(x, y, COLORS[color]);
}
