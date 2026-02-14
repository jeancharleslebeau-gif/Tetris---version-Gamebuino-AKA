/*
===============================================================================
  game.cpp — Machine à états du Tetris (AKA Edition)
-------------------------------------------------------------------------------
  Rôle :
    - Gérer les états : Title, Playing, Win, Dead
    - Appeler la logique (game_update_logic)
    - Appeler le rendu (render_game)
    - Gérer les transitions (start, win, dead)
===============================================================================
*/

#include "game.h"
#include "game_state.h"
#include "game_logic.h"
#include "render.h"
#include "input.h"
#include "audio.h"

// -----------------------------------------------------------------------------
//  État courant du jeu
// -----------------------------------------------------------------------------
static GameMode g_mode = GameMode::Title;
static GameMode g_prevMode = GameMode::Title;

// -----------------------------------------------------------------------------
//  Accès global
// -----------------------------------------------------------------------------
GameMode& game_mode()
{
    return g_mode;
}

// -----------------------------------------------------------------------------
//  Actions à effectuer lors d’un changement d’état
// -----------------------------------------------------------------------------
static void on_enter_mode(GameMode m)
{
    switch(m)
    {
        case GameMode::Title:
            // rien de spécial
            break;

        case GameMode::Playing:
            logic_reset();   // reset complet du tetris
            break;

        case GameMode::Win:
            break;

        case GameMode::Dead:
            break;
    }
}

// -----------------------------------------------------------------------------
//  Initialisation du jeu
// -----------------------------------------------------------------------------
void game_init()
{
    g_mode = GameMode::Title;
    g_prevMode = GameMode::Title;
    on_enter_mode(g_mode);
}

// -----------------------------------------------------------------------------
//  Logique principale (appelée à chaque frame)
// -----------------------------------------------------------------------------
void game_update()
{
    // Détection de changement d’état
    if(g_mode != g_prevMode)
    {
        on_enter_mode(g_mode);
        g_prevMode = g_mode;
    }

    switch(g_mode)
    {
        case GameMode::Title:
            if(button_pressed(BTN_A))
            {
                click();
                g_mode = GameMode::Playing;
            }
            break;

        case GameMode::Playing:
        {
            game_update_logic();

            auto& S = game_state();
            if(S.hasDied)
                g_mode = GameMode::Dead;

            break;
        }

        case GameMode::Dead:
            if(button_pressed(BTN_A))
            {
                click();
                g_mode = GameMode::Playing;
                logic_reset();   // <<< AJOUT ESSENTIEL
            }
            break;

        case GameMode::Win:
            if(button_pressed(BTN_A))
            {
                click();
                g_mode = GameMode::Playing;
                logic_reset();
            }
            break;
    }
}

// -----------------------------------------------------------------------------
//  Rendu (appelé à chaque frame)
// -----------------------------------------------------------------------------
void game_render()
{
    switch(g_mode)
    {
        case GameMode::Title:
            render_title();
            break;

        case GameMode::Playing:
            render_game();
            break;

        case GameMode::Dead:
            render_game();
            render_game_over();
            break;

        case GameMode::Win:
            render_game();
            break;
    }
}
