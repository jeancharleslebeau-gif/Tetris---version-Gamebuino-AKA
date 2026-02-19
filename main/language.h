/*
===============================================================================
  language.h — Gestion multilingue (Tetris AKA)
-------------------------------------------------------------------------------
  Rôle :
    - Définir les langues supportées.
    - Définir les identifiants de textes (TextID).
    - Fournir l’API pour changer de langue, récupérer la langue courante,
      traduire un texte (tr), et gérer la persistance via NVS.

  Auteur : Jean‑Charles LEBEAU (Jicehel)
  Création : 02/2026
===============================================================================
*/

#pragma once

// -----------------------------------------------------------------------------
//  Langues supportées
// -----------------------------------------------------------------------------
enum class Language {
    FR = 0,
    EN,
    DE,
    IT,
    ES,
    PT,
    DK,
    NL,

    COUNT
};

// -----------------------------------------------------------------------------
//  Identifiants de textes (UI 100 % traduite)
// -----------------------------------------------------------------------------
enum class TextID {

    // Menu principal
    MENU_PLAY,
    MENU_OPTIONS,
    MENU_QUIT,

    // Écran titre
    TITLE_PRESS_A,
    PRESS_A_START,
    PRESS_A_CONTINUE,

    // Pause
    PAUSE_TITLE,
    PAUSE_MUSIC_LABEL,
    PAUSE_SFX_LABEL,
    PAUSE_LANGUAGE_LABEL,
    PAUSE_BACK_TO_GAME,
    PAUSE_BACK_TO_MENU,

    // Game Over
    GAME_OVER,
    HISCORE,

    // HUD
    HUD_TITLE_TETRIS,
    HUD_SCORE,
    HUD_LINES,
    HUD_LEVEL,
    HUD_NEXT,

    // Orientation écran (menu Pause)
    PAUSE_ORIENTATION_LABEL,
    PAUSE_ORIENTATION_ON,
    PAUSE_ORIENTATION_OFF,
	
	// Écran titre — aides 
	TITLE_HINT_CHANGE_LANG, 
	TITLE_HINT_PLAY,
};


// -----------------------------------------------------------------------------
//  API principale
// -----------------------------------------------------------------------------

void language_set(Language lang);
Language language_get();

void language_next();
void language_prev();

const char* tr(TextID id);
const char* language_name(Language lang);

// -----------------------------------------------------------------------------
//  Persistance NVS
// -----------------------------------------------------------------------------

void language_load_from_nvs();
void language_save_to_nvs();
