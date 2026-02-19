/*
===============================================================================
  game.cpp — Boucle principale et gestion des états (Tetris AKA)
-------------------------------------------------------------------------------
  Rôle :
    - Gérer les états du jeu :
        * Titre
        * En jeu
        * Pause
        * Game Over
    - Gérer les inputs (rotés en portrait pour le mode vertical)
    - Gérer la rotation écran (orientation)
    - Gérer le changement de langue
    - Appeler le rendu (render.cpp)
===============================================================================
*/

#include "game.h"
#include "game_state.h"
#include "game_logic.h"
#include "render.h"
#include "input_rotated.h"
#include "language.h"
#include "config.h"
#include "graphics_rotated.h"
#include "graphics.h"
#include "audio.h"

extern gb_core g_core;

// -----------------------------------------------------------------------------
// État global
// -----------------------------------------------------------------------------
static GameMode g_mode = GameMode::Title;

GameMode& game_mode()
{
    return g_mode;
}

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------
static void start_new_game()
{
    logic_reset();
    audio_play_music(MUSIC_GAME);
    g_mode = GameMode::Playing;
}


static void go_to_title()
{
	audio_play_music(MUSIC_MENU);
    g_mode = GameMode::Title;
}

// -----------------------------------------------------------------------------
// Menu Pause
// -----------------------------------------------------------------------------
int pause_selection = 0;

static void update_pause()
{
    // Sélecteur de fonction de lecture des boutons selon orientation
    auto pressed = g_rotate_screen ? button_pressed_rot : button_pressed;

    // Navigation (6 options : 0..5)
    if (pressed(BTN_UP))
        pause_selection = (pause_selection - 1 + 6) % 6;

    if (pressed(BTN_DOWN))
        pause_selection = (pause_selection + 1) % 6;

    // Gauche = valeur précédente
    if (pressed(BTN_LEFT))
    {
        switch (pause_selection)
        {
            case 0:
                volume_music = (volume_music + 10) % 11;
                audio_set_music_volume(volume_music);
                break;

            case 1:
                volume_sfx = (volume_sfx + 10) % 11;
                audio_set_sfx_volume(volume_sfx);
                break;

            case 2: language_prev(); break;

            case 3:
                g_rotate_screen = !g_rotate_screen;
                save_config();
                gfx_clear(COLOR_BLACK);   // <-- IMPORTANT
                break;
        }
    }

    // Droite = valeur suivante
    if (pressed(BTN_RIGHT))
    {
        switch (pause_selection)
        {
            case 0:
                volume_music = (volume_music + 1) % 11;
                audio_set_music_volume(volume_music);
                break;

            case 1:
                volume_sfx = (volume_sfx + 1) % 11;
                audio_set_sfx_volume(volume_sfx);
                break;

            case 2: language_next(); break;

            case 3:
                g_rotate_screen = !g_rotate_screen;
                save_config();
                gfx_clear(COLOR_BLACK);   // <-- IMPORTANT
                break;
        }
    }

    // A = valider (reprendre / retour menu)
	if (pressed(BTN_A))
	{
		if (pause_selection == 4) { 
			audio_play_music(MUSIC_GAME);
			g_mode = GameMode::Playing; 
			return; 
		}
		if (pause_selection == 5) { 
			go_to_title(); 
			return; 
		}
	}

    // B = retour menu direct
    if (pressed(BTN_B))
    {
        go_to_title();
        return;
    }
}

// -----------------------------------------------------------------------------
// Mise à jour du jeu en cours
// -----------------------------------------------------------------------------
static void update_playing()
{
    auto& S = game_state();

    // Pause
    if (button_pressed(BTN_MENU))
    {
		audio_stop_music();
        g_mode = GameMode::Paused;
        pause_selection = 0;
        return;
    }

    // -------------------------------------------------------------------------
    // MODE HORIZONTAL (non roté) — mapping actuel (inchangé)
    // -------------------------------------------------------------------------
    if (!g_rotate_screen)
    {
        if (button_pressed(BTN_LEFT))
            game_move_left();

        if (button_pressed(BTN_RIGHT))
            game_move_right();

        // A : slow drop
        if (button_pressed(BTN_A))
            game_soft_drop();

        // BAS : slow drop
        if (button_pressed(BTN_DOWN))
            game_soft_drop();

        // D : fast drop
        if (button_pressed(BTN_D))
            game_hard_drop();

        // C : rotation anti-horaire
        if (button_pressed(BTN_C))
            game_rotate_ccw();

        // B : rotation horaire
        if (button_pressed(BTN_B))
            game_rotate_cw();
    }

    // -------------------------------------------------------------------------
    // MODE VERTICAL (roté) — nouveau mapping joystick
    // -------------------------------------------------------------------------
    else
    {
        // Lecture joystick analogique
        int rawY = g_core.joystick.get_y();
        const int dead = 150;
        int joyY = 0;
        if (rawY >  dead) joyY = +1;
        if (rawY < -dead) joyY = -1;

        // --- Déplacement latéral via joystick ---
        // Joystick vers le haut → gauche
        if (joyY == +1)
            game_move_left();

        // Joystick vers le bas → droite
        if (joyY == -1)
            game_move_right();

        // --- Slow drop ---
 
        // D-Pad roté (DOWN roté = slow drop)
        if (button_down_rot(BTN_DOWN))
            game_soft_drop();
			
		if (g_core.buttons.pressed(gb_buttons::KEY_C))
			game_soft_drop();	

        // --- Hard drop ---
        // Bouton A
        if (g_core.buttons.pressed(gb_buttons::KEY_B))
            game_hard_drop();

        // --- Rotations ---
        if (button_pressed_rot(BTN_LEFT))
            game_rotate_ccw();

        if (button_pressed_rot(BTN_RIGHT))
            game_rotate_cw();
    }

    // Tick logique
    game_tick();

	// Game Over ?
	if (S.hasDied)
	{
		audio_stop_music();
		audio_game_over_sfx();
		audio_play_music(MUSIC_GAMEOVER);
		g_mode = GameMode::GameOver;
	}

    // Rendu jeu
    render_game();
}

// -----------------------------------------------------------------------------
// Mise à jour Game Over
// -----------------------------------------------------------------------------
static void update_game_over()
{
    render_game_over();

    if (button_pressed(BTN_A) || button_pressed(BTN_MENU))
        go_to_title();
}

// -----------------------------------------------------------------------------
// Mise à jour écran titre
// -----------------------------------------------------------------------------
static void update_title()
{
    render_title(); // image de fond

    auto lang = language_get();

    // Ligne 1 : "Langue : Français"
    char buf[64];
    sprintf(buf, "%s : %s",
            tr(TextID::PAUSE_LANGUAGE_LABEL),
            language_name(lang));

    gfx_text_rot(10, SCREEN_H - 40, buf, COLOR_WHITE);

    // Ligne 2 : "< > : changer de langue   A : Jouer"
    char hint[96];
    sprintf(hint, "%s   %s",
            tr(TextID::TITLE_HINT_CHANGE_LANG),
            tr(TextID::TITLE_HINT_PLAY));

    gfx_text_rot(10, SCREEN_H - 20, hint, COLOR_GRAY);

    // Démarrer
    if (button_pressed(BTN_A))
        start_new_game();

    // Changer langue
    if (button_pressed(BTN_LEFT))
        language_prev();

    if (button_pressed(BTN_RIGHT))
        language_next();
}

// -----------------------------------------------------------------------------
// Boucle principale
// -----------------------------------------------------------------------------
void game_update()
{
    switch (g_mode)
    {
        case GameMode::Title:
            update_title();
            break;

        case GameMode::Playing:
            update_playing();
            break;

        case GameMode::Paused:
            update_pause();
            break;

        case GameMode::GameOver:
            update_game_over();
            break;
    }
}

// -----------------------------------------------------------------------------
// Initialisation du jeu (appelée par task_game)
// -----------------------------------------------------------------------------
void game_init()
{
    // Reset complet
    game_state_reset();
    logic_reset();

    // État initial
    g_mode = GameMode::Title;

    // Charger langue + config
    language_load_from_nvs();
    load_config();

    // --- AUDIO ---
    audio_init();
    audio_set_music_volume(volume_music);
    audio_set_sfx_volume_silent(volume_sfx);
    audio_play_music(MUSIC_MENU);
}


// -----------------------------------------------------------------------------
// Rendu global (appelé par task_game)
// -----------------------------------------------------------------------------
void game_render()
{
    switch (g_mode)
    {
        case GameMode::Title:
            render_title();
            break;

        case GameMode::Playing:
            render_game();
            break;

        case GameMode::Paused:
            render_pause_menu();
            gfx_flush();
            break;

        case GameMode::GameOver:
            render_game_over();
            break;
    }
}
