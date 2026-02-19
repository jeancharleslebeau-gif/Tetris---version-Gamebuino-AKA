/*
===============================================================================
  game_logic.cpp — Logique Tetris (AKA Edition — Version découplée)
-------------------------------------------------------------------------------
  Rôle :
    - Logique pure du Tetris :
        * collisions
        * rotation SRS
        * gravité
        * lock delay
        * clear lines + scoring
        * level up
        * next piece
        * hard drop / soft drop
    - Aucune lecture des touches
    - Aucune dépendance à l’orientation
    - Aucune dépendance au rendu
    - API appelée par game.cpp
===============================================================================
*/

#include "game_logic.h"
#include "game_state.h"
#include "pieces.h"
#include "audio.h"
#include "audio_pmf.h"
#include "esp_timer.h"
#include "esp_random.h"
#include <string.h>

// -----------------------------------------------------------------------------
// Constantes
// -----------------------------------------------------------------------------
static int  GRAVITY_INTERVAL_MS = 500;
static constexpr int LOCK_DELAY_MS = 500;

// -----------------------------------------------------------------------------
// Grille interne 20×10
// -----------------------------------------------------------------------------
static uint8_t grid[TETRIS_ROWS][TETRIS_COLS];

// -----------------------------------------------------------------------------
// Pièce active
// -----------------------------------------------------------------------------
static int curPiece;
static int curRot;
static int curX;
static int curY;

static int64_t lockStartTime   = -1;
static int64_t lastGravityTime = 0;

// -----------------------------------------------------------------------------
// Collision
// -----------------------------------------------------------------------------
static bool collide(int x, int y, int rot)
{
    for (int py = 0; py < 4; py++)
    for (int px = 0; px < 4; px++)
    {
        if (!piece_at(curPiece, rot, py, px))
            continue;

        int gx = x + px;
        int gy = y + py;

        if (gx < 0 || gx >= TETRIS_COLS) return true;
        if (gy >= TETRIS_ROWS)          return true;
        if (gy >= 0 && grid[gy][gx])    return true;
    }
    return false;
}

// -----------------------------------------------------------------------------
// Rotation SRS
// -----------------------------------------------------------------------------
static bool try_rotate(int dir)
{
    int oldRot = curRot;
    int newRot = (curRot + dir + 4) & 3;

    const int8_t (*table)[5][2] =
        (curPiece == PIECE_I) ? SRS_I : SRS_JLSTZ;

    for (int i = 0; i < 5; i++)
    {
        int nx = curX + table[oldRot][i][0];
        int ny = curY + table[oldRot][i][1];

        if (!collide(nx, ny, newRot))
        {
            curX = nx;
            curY = ny;
            curRot = newRot;
            lockStartTime = -1;
            audio_rotate();
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------
// Nouvelle pièce
// -----------------------------------------------------------------------------
static void new_piece()
{
    auto& S = game_state();

    curPiece = S.nextPiece;
    curRot   = 0;
    curX     = 3;
    curY     = -2;
    lockStartTime = -1;

    S.nextPiece = esp_random() % 7;

    S.currentPiece = curPiece;
    S.currentRot   = curRot;
    S.currentX     = curX;
    S.currentY     = curY;
}

// -----------------------------------------------------------------------------
// Lock
// -----------------------------------------------------------------------------
static void lock_piece()
{
    auto& S = game_state();
    bool died = false;

    for (int py = 0; py < 4; py++)
    for (int px = 0; px < 4; px++)
    {
        if (!piece_at(curPiece, curRot, py, px))
            continue;

        int gx = curX + px;
        int gy = curY + py;

        if (gy < 0)
        {
            died = true;
            continue;
        }

        grid[gy][gx] = curPiece + 1;
    }

    audio_lock();

    if (died)
    {
        S.hasDied = true;
        audio_game_over_sfx();
    }
}

// -----------------------------------------------------------------------------
// Clear lines + scoring + level up
// -----------------------------------------------------------------------------
static void clear_lines()
{
    int cleared = 0;

    for (int y = TETRIS_ROWS - 1; y >= 0; y--)
    {
        bool full = true;
        for (int x = 0; x < TETRIS_COLS; x++)
        {
            if (!grid[y][x]) { full = false; break; }
        }

        if (full)
        {
            cleared++;
            for (int yy = y; yy > 0; yy--)
                memcpy(grid[yy], grid[yy - 1], TETRIS_COLS);
            memset(grid[0], 0, TETRIS_COLS);
            y++;
        }
    }

    if (cleared > 0)
    {
        auto& S = game_state();

        S.linesCleared += cleared;
        S.score        += cleared * 100;

        audio_line_clear(cleared);

        int newLevel = 1 + S.linesCleared / 10;

        if (newLevel != S.currentLevel)
        {
            S.currentLevel = newLevel;
            audio_level_up();

            GRAVITY_INTERVAL_MS = 500 - (newLevel - 1) * 40;
            if (GRAVITY_INTERVAL_MS < 80)
                GRAVITY_INTERVAL_MS = 80;

            audio_pmf_set_level(S.currentLevel);
        }
    }
}

// -----------------------------------------------------------------------------
// API — Appelée par game.cpp
// -----------------------------------------------------------------------------
void game_move_left()
{
    if (!collide(curX - 1, curY, curRot))
    {
        curX--;
        lockStartTime = -1;
        audio_move();
    }
}

void game_move_right()
{
    if (!collide(curX + 1, curY, curRot))
    {
        curX++;
        lockStartTime = -1;
        audio_move();
    }
}

// Rotation horaire (CW)
void game_rotate_cw()
{
    try_rotate(+1);
}

// Rotation anti-horaire (CCW)
void game_rotate_ccw()
{
    try_rotate(-1);
}

// Compatibilité ancienne API : rotation horaire
void game_rotate()
{
    game_rotate_cw();
}

void game_soft_drop()
{
    if (!collide(curX, curY + 1, curRot))
    {
        curY++;
        lockStartTime = -1;
        audio_soft_drop();
    }
}

void game_hard_drop()
{
    auto& S = game_state();

    while (!collide(curX, curY + 1, curRot))
        curY++;

    audio_hard_drop();

    lock_piece();
    if (S.hasDied) return;

    clear_lines();
    new_piece();
}

// -----------------------------------------------------------------------------
// Tick logique (gravité + lock + clear lines)
// -----------------------------------------------------------------------------
void game_tick()
{
    auto& S = game_state();

    if (S.hasDied)
        return;

    int64_t now = esp_timer_get_time() / 1000;

    // Gravité
    if (now - lastGravityTime >= GRAVITY_INTERVAL_MS)
    {
        lastGravityTime = now;

        if (!collide(curX, curY + 1, curRot))
        {
            curY++;
            lockStartTime = -1;
        }
        else
        {
            if (lockStartTime < 0)
                lockStartTime = now;

            if (now - lockStartTime >= LOCK_DELAY_MS)
            {
                lock_piece();
                if (S.hasDied) return;

                clear_lines();
                new_piece();
            }
        }
    }

    // Sync vers GameState
    S.currentPiece = curPiece;
    S.currentRot   = curRot;
    S.currentX     = curX;
    S.currentY     = curY;

    for (int y = 0; y < TETRIS_ROWS; ++y)
    for (int x = 0; x < TETRIS_COLS; ++x)
        S.field[y][x] = grid[y][x];
}

// -----------------------------------------------------------------------------
// Reset complet
// -----------------------------------------------------------------------------
void logic_reset()
{
    memset(grid, 0, sizeof(grid));

    auto& S = game_state();
    memset(S.field, 0, sizeof(S.field));

    S.currentLevel = 1;
    S.score        = 0;
    S.linesCleared = 0;
    S.hasDied      = false;
    S.hasWon       = false;

    audio_pmf_set_level(S.currentLevel);

    S.nextPiece = esp_random() % 7;

    curPiece = S.nextPiece;
    curRot   = 0;
    curX     = 3;
    curY     = -2;

    lockStartTime   = -1;
    lastGravityTime = esp_timer_get_time() / 1000;

    S.currentPiece = curPiece;
    S.currentRot   = curRot;
    S.currentX     = curX;
    S.currentY     = curY;
}
