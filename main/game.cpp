/*
===============================================================================
  game.cpp — Gestion du jeu Tetris (AKA Edition)
-------------------------------------------------------------------------------
  Rôle :
    - Gérer les états simples : MENU, GAME, PAUSE, GAME OVER
    - Appeler game_logic.cpp pour la logique Tetris
    - Jouer / arrêter la musique selon l’état
    - Afficher les écrans via render.cpp
===============================================================================
*/

#include "game.h"
#include "graphics.h"
#include "input.h"
#include "game_logic.h"
#include "game_state.h"
#include "audio.h"
#include "render.h"
#include "audio_pmf.h"

// Volumes utilisateur
static uint8_t volume_music = 180;   // 0..255
static uint8_t volume_sfx   = 6;     // 0..8

// -----------------------------------------------------------------------------
// États simples
// -----------------------------------------------------------------------------

enum GameScene {
    SCENE_MENU,
    SCENE_GAME,
    SCENE_PAUSE,
    SCENE_GAMEOVER
};

static GameScene scene = SCENE_MENU;

// -----------------------------------------------------------------------------
// Initialisation du jeu
// -----------------------------------------------------------------------------

void game_init()
{
    scene = SCENE_MENU;

    audio_pmf_init();                 // initialise le player PMF
    audio_pmf_set_master_volume(volume_music);  // applique le volume musique
    audio_set_volume(volume_sfx);               // applique le volume SFX

    audio_pmf_play(MusicID::MENU, true);        // maintenant on peut jouer
}

// -----------------------------------------------------------------------------
// Update
// -----------------------------------------------------------------------------

void game_update()
{
    auto& S = game_state();

    switch (scene)
    {
        // ---------------------------------------------------------------------
        // MENU
        // ---------------------------------------------------------------------
        case SCENE_MENU:
            if (button_pressed(BTN_A))
            {
                logic_reset();
                audio_pmf_play(MusicID::GAME, true);
                scene = SCENE_GAME;
            }
            break;

        // ---------------------------------------------------------------------
        // JEU
        // ---------------------------------------------------------------------
        case SCENE_GAME:

            if (button_pressed(BTN_MENU))
            {
                audio_pmf_stop();
                scene = SCENE_PAUSE;
                return;
            }

            game_update_logic();

            if (S.hasDied)
            {
                audio_pmf_stop();
                audio_game_over_sfx();
                audio_pmf_play(MusicID::GAMEOVER, false);
                scene = SCENE_GAMEOVER;
                return;
            }
            break;

        // ---------------------------------------------------------------------
        // PAUSE
        // ---------------------------------------------------------------------
        case SCENE_PAUSE:

            // Volume musique
            if (button_pressed(BTN_RIGHT))
            {
                if (volume_music < 255) volume_music += 5;
                audio_pmf_set_master_volume(volume_music);
            }
            if (button_pressed(BTN_LEFT))
            {
                if (volume_music > 0) volume_music -= 5;
                audio_pmf_set_master_volume(volume_music);
            }

            // Volume SFX
            if (button_pressed(BTN_UP))
            {
                if (volume_sfx < 8) volume_sfx++;
                audio_set_volume(volume_sfx);
            }
            if (button_pressed(BTN_DOWN))
            {
                if (volume_sfx > 0) volume_sfx--;
                audio_set_volume(volume_sfx);
            }

            // Navigation
            if (button_pressed(BTN_A))
            {
                audio_pmf_play(MusicID::GAME, true);
                scene = SCENE_GAME;
            }
            if (button_pressed(BTN_B))
            {
                logic_reset();
                audio_pmf_play(MusicID::MENU, true);
                scene = SCENE_MENU;
            }
            break;

        // ---------------------------------------------------------------------
        // GAME OVER
        // ---------------------------------------------------------------------
        case SCENE_GAMEOVER:
            if (button_pressed(BTN_A))
            {
                logic_reset();
                audio_pmf_play(MusicID::MENU, true);
                scene = SCENE_MENU;
            }
            break;
    }
}

// -----------------------------------------------------------------------------
// Render
// -----------------------------------------------------------------------------

void game_render()
{
    switch (scene)
    {
        case SCENE_MENU:
            render_title();
            break;

        case SCENE_GAME:
            render_game();
            break;

        case SCENE_PAUSE:
        {
            gfx_clear(COLOR_BLACK);
            gfx_text_center(60, "PAUSE", COLOR_WHITE);

            char buf[32];

            sprintf(buf, "Musique : %d", volume_music);
            gfx_text_center(120, buf, COLOR_WHITE);

            sprintf(buf, "SFX : %d", volume_sfx);
            gfx_text_center(150, buf, COLOR_WHITE);

            gfx_text_center(200, "A = Reprendre", COLOR_WHITE);
            gfx_text_center(220, "B = Menu", COLOR_WHITE);

            gfx_flush();
            break;
        }

        case SCENE_GAMEOVER:
            render_game_over();
            break;
    }
}
