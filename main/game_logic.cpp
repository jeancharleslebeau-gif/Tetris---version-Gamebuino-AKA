/*
===============================================================================
  game_logic.cpp — Tetris Logic (AKA Edition — Version SRS + Lock Delay)
-------------------------------------------------------------------------------
  Inclus :
    - Collision sol / murs / blocs
    - Lock delay (500 ms)
    - Gravité indépendante du framerate
    - Soft drop (↓)
    - Hard drop (↑)
    - Rotation SRS Guideline (I + JLSTZ)
    - Clear lines + scoring
    - Nouvelle pièce + preview
    - Synchronisation vers GameState (rendu)
    - Utilise pieces.h comme source unique des pièces
===============================================================================
*/

#include "game_logic.h"
#include "input.h"
#include "graphics.h"
#include "game_state.h"
#include "pieces.h"        // ⭐ Source unique des pièces + kicks SRS
#include "esp_timer.h"
#include "esp_random.h"
#include <string.h>

// -----------------------------------------------------------------------------
// Constantes
// -----------------------------------------------------------------------------
static constexpr int LOCK_DELAY_MS       = 500;
static int GRAVITY_INTERVAL_MS = 500;   // initialisation de la vitesse pour le niveau 1
static constexpr int SOFT_DROP_SPEED     = 1;

// -----------------------------------------------------------------------------
// Grille logique interne (20×10)
// -----------------------------------------------------------------------------
static uint8_t grid[TETRIS_ROWS][TETRIS_COLS];

// -----------------------------------------------------------------------------
// État courant de la pièce
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

    // La pièce courante devient la "nextPiece"
    curPiece = S.nextPiece;
    curRot   = 0;
    curX     = 3;
    curY     = -2;
    lockStartTime = -1;

    // Tirage de la prochaine pièce
    S.nextPiece = esp_random() % 7;

    // Synchronisation immédiate
    S.currentPiece = curPiece;
    S.currentRot   = curRot;
    S.currentX     = curX;
    S.currentY     = curY;

    // Game Over si spawn impossible
    if (collide(curX, curY, curRot))
    {
        memset(grid, 0, sizeof(grid));
        memset(S.field, 0, sizeof(S.field));
        S.hasDied = true;
        return;
    }
}

// -----------------------------------------------------------------------------
// Lock
// -----------------------------------------------------------------------------
static void lock_piece()
{
    for (int py = 0; py < 4; py++)
    for (int px = 0; px < 4; px++)
    {
        if (!piece_at(curPiece, curRot, py, px))
            continue;

        int gx = curX + px;
        int gy = curY + py;

        if (gy >= 0)
            grid[gy][gx] = curPiece + 1;
    }
}

// -----------------------------------------------------------------------------
// Clear lines + scoring
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

        // Passage de niveau : toutes les 10 lignes
        int newLevel = 1 + S.linesCleared / 10;

        if (newLevel != S.currentLevel)
        {
            S.currentLevel = newLevel;

            // Accélération de la gravité (simple, stable)
            // Exemple : niveau 1 = 500ms, niveau 2 = 450ms, etc.
            // Minimum 80ms pour éviter l'insta-drop
            int newGravity = 500 - (newLevel - 1) * 50;
            if (newGravity < 80)
                newGravity = 80;

            // On met à jour la variable globale
            GRAVITY_INTERVAL_MS = newGravity;
        }
    }
}


// -----------------------------------------------------------------------------
// Hard drop
// -----------------------------------------------------------------------------
static void hard_drop()
{
    while (!collide(curX, curY + 1, curRot))
        curY++;

    lock_piece();
    clear_lines();
    new_piece();
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

    // Première nextPiece
    S.nextPiece = esp_random() % 7;

    // Première pièce courante
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

// -----------------------------------------------------------------------------
// Logique principale
// -----------------------------------------------------------------------------
void game_update_logic()
{
    int64_t now = esp_timer_get_time() / 1000;

    // --- Déplacements horizontaux ---
    if (button_pressed(BTN_LEFT) && !collide(curX - 1, curY, curRot))
    {
        curX--;
        lockStartTime = -1;
    }

    if (button_pressed(BTN_RIGHT) && !collide(curX + 1, curY, curRot))
    {
        curX++;
        lockStartTime = -1;
    }

    // --- Rotation ---
    if (button_pressed(BTN_A))
        try_rotate(+1);

    // --- Hard drop ---
    if (button_pressed(BTN_UP))
    {
        hard_drop();
        goto sync_state;
    }

    // --- Soft drop ---
    if (button_down(BTN_DOWN))
    {
        if (!collide(curX, curY + 1, curRot))
        {
            curY++;
            lockStartTime = -1;
        }
        else
            goto handle_lock;

        goto sync_state;
    }

    // --- Gravité normale ---
    if (now - lastGravityTime >= GRAVITY_INTERVAL_MS)
    {
        lastGravityTime = now;

        if (!collide(curX, curY + 1, curRot))
        {
            curY++;
            lockStartTime = -1;
        }
        else
            goto handle_lock;
    }

    goto sync_state;

    // --- Lock delay ---
handle_lock:
    if (lockStartTime < 0)
        lockStartTime = now;

    if (now - lockStartTime >= LOCK_DELAY_MS)
    {
        lock_piece();
        clear_lines();
		// Détection de pile trop haute
		for (int x = 0; x < TETRIS_COLS; x++)
		{
			if (grid[0][x] != 0)   // ligne du haut
			{
				auto& S = game_state();
				S.hasDied = true;
				return;
			}
		}
        new_piece();
    }

    // --- Synchronisation vers GameState ---
sync_state:
    auto& S = game_state();

    S.currentPiece = curPiece;
    S.currentRot   = curRot;
    S.currentX     = curX;
    S.currentY     = curY;

    for (int y = 0; y < TETRIS_ROWS; ++y)
        for (int x = 0; x < TETRIS_COLS; ++x)
            S.field[y][x] = grid[y][x];
}
