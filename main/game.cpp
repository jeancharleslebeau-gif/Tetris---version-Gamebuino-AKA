/*
===============================================================================
  game.cpp — Gestion du jeu Tetris (AKA Edition)
-------------------------------------------------------------------------------
  Rôle :
    - Gérer les états simples : MENU, GAME, PAUSE, GAME OVER
    - Appeler game_logic.cpp pour la logique Tetris
    - Jouer / arrêter la musique selon l’état
    - Afficher les écrans via render.cpp
    - Gérer les volumes audio et la langue dans le menu PAUSE
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
#include "language.h"
#include "flags.h"

// -----------------------------------------------------------------------------
// Volumes utilisateur (0..10)
// -----------------------------------------------------------------------------
static uint8_t volume_music = 7;
static uint8_t volume_sfx   = 6;

// Index du menu pause :
// 0 = musique
// 1 = SFX
// 2 = langue
static int pause_menu_index = 0;

// -----------------------------------------------------------------------------
// États simples du jeu
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
    volume_music = 7;
    volume_sfx   = 6;
    pause_menu_index = 0;

    audio_init();

    audio_set_music_volume(volume_music);
    audio_set_sfx_volume_silent(volume_sfx);

    audio_play_music(MUSIC_MENU);

    logic_reset();
}

// -----------------------------------------------------------------------------
// Update — logique principale selon la scène
// -----------------------------------------------------------------------------
void game_update()
{
    auto& S = game_state();

    switch (scene)
    {
        // ---------------------------------------------------------------------
        // MENU PRINCIPAL
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
        // JEU EN COURS
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
        {
            // Navigation entre les lignes
            if (button_pressed(BTN_UP))
            {
                if (pause_menu_index > 0)
                    pause_menu_index--;
            }
            if (button_pressed(BTN_DOWN))
            {
                if (pause_menu_index < 2)
                    pause_menu_index++;
            }

            // Modification des valeurs
            if (button_pressed(BTN_RIGHT))
            {
                if (pause_menu_index == 0)   // Volume musique
                {
                    if (volume_music < 10)
                        volume_music++;

                    audio_set_music_volume(volume_music);
                }
                else if (pause_menu_index == 1) // Volume SFX
                {
                    if (volume_sfx < 10)
                        volume_sfx++;

                    audio_set_sfx_volume(volume_sfx);
                }
                else if (pause_menu_index == 2) // Langue
                {
                    language_next();
                }
            }

            if (button_pressed(BTN_LEFT))
            {
                if (pause_menu_index == 0)
                {
                    if (volume_music > 0)
                        volume_music--;

                    audio_set_music_volume(volume_music);
                }
                else if (pause_menu_index == 1)
                {
                    if (volume_sfx > 0)
                        volume_sfx--;

                    audio_set_sfx_volume(volume_sfx);
                }
                else if (pause_menu_index == 2)
                {
                    language_prev();
                }
            }

            // Reprendre
            if (button_pressed(BTN_A))
            {
                audio_play_music(MUSIC_GAME);
                scene = SCENE_GAME;
            }

            // Retour menu principal
            if (button_pressed(BTN_B))
            {
                logic_reset();
                audio_play_music(MUSIC_MENU);
                scene = SCENE_MENU;
            }
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
// Render — délégué à render.cpp sauf pour le menu de Pause
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

			// Titre Pause (remonté de 10 px)
			gfx_text_center(50, tr(TextID::PAUSE_TITLE), COLOR_WHITE);

			// Position des lignes (remontées de 30 px)
			const int y_music  = 90;
			const int y_sfx    = 120;
			const int y_lang   = 150;

			// Curseur
			int cursor_y = (pause_menu_index == 0) ? y_music :
						   (pause_menu_index == 1) ? y_sfx : y_lang;

			gfx_text(40, cursor_y, ">", COLOR_YELLOW);

			// Affichage des valeurs
			char buf[32];

			sprintf(buf, "%s : %d", tr(TextID::PAUSE_MUSIC_LABEL), volume_music);
			gfx_text(60, y_music, buf, COLOR_WHITE);

			sprintf(buf, "%s : %d", tr(TextID::PAUSE_SFX_LABEL), volume_sfx);
			gfx_text(60, y_sfx, buf, COLOR_WHITE);

			// Langue
			gfx_text(60, y_lang, tr(TextID::PAUSE_LANGUAGE_LABEL), COLOR_WHITE);

			int flag_index = (int)language_get();
			gfx_blitRegion(
				flags_atlas,
				FLAGS_ATLAS_WIDTH,
				flag_index * FLAG_WIDTH, 0,
				FLAG_WIDTH, FLAG_HEIGHT,
				160, y_lang - 4
			);

			gfx_text(200, y_lang, language_name(language_get()), COLOR_WHITE);

			// Boutons fixes A: et B:
			char bufA[32];
			sprintf(bufA, "A: %s", tr(TextID::PAUSE_BACK_TO_GAME));
			gfx_text_center(200, bufA, COLOR_WHITE);

			char bufB[32];
			sprintf(bufB, "B: %s", tr(TextID::PAUSE_BACK_TO_MENU));
			gfx_text_center(220, bufB, COLOR_WHITE);

			gfx_flush();
        }
        break;

        case SCENE_GAMEOVER:
            render_game_over();
            break;
    }
}
