/*
===============================================================================
  language.cpp — Gestion multilingue (Tetris AKA)
-------------------------------------------------------------------------------
  Rôle :
    - Tables de traduction
    - API tr()
    - Gestion de la langue courante
    - Persistance NVS
===============================================================================
*/

#include "language.h"
#include <nvs_flash.h>
#include <nvs.h>

// -----------------------------------------------------------------------------
//  Langue courante (fallback = anglais)
// -----------------------------------------------------------------------------
static Language g_lang = Language::EN;

// -----------------------------------------------------------------------------
//  Tables de traduction
//  IMPORTANT : ordre strict = ordre de TextID
// -----------------------------------------------------------------------------

// --- Français ----------------------------------------------------------------
static const char* FR[] = {
    "Jouer",
    "Options",
    "Quitter",

    "PAUSE",
    "Musique",
    "SFX",
    "Langue",
    "Reprendre",
    "Retour au menu",

    "GAME OVER",
    "Meilleur score",

    "TETRIS",
    "Score :",
    "Lignes :",
    "Niveau :",
    "Suivant :",
};

// --- Anglais -----------------------------------------------------------------
static const char* EN[] = {
    "Play",
    "Options",
    "Quit",

    "PAUSE",
    "Music",
    "SFX",
    "Language",
    "Resume",
    "Back to menu",

    "GAME OVER",
    "High score",

    "TETRIS",
    "Score:",
    "Lines:",
    "Level:",
    "Next:",
};

// --- Allemand ----------------------------------------------------------------
static const char* DE[] = {
    "Spielen",
    "Optionen",
    "Beenden",

    "PAUSE",
    "Musik",
    "SFX",
    "Sprache",
    "Fortsetzen",
    "Zum Menü",

    "GAME OVER",
    "Rekord",

    "TETRIS",
    "Punkte:",
    "Linien:",
    "Level:",
    "Nächste:",
};

// --- Italien -----------------------------------------------------------------
static const char* IT[] = {
    "Gioca",
    "Opzioni",
    "Esci",

    "PAUSA",
    "Musica",
    "SFX",
    "Lingua",
    "Riprendi",
    "Menu",

    "GAME OVER",
    "Record",

    "TETRIS",
    "Punteggio:",
    "Linee:",
    "Livello:",
    "Prossimo:",
};

// --- Espagnol ----------------------------------------------------------------
static const char* ES[] = {
    "Jugar",
    "Opciones",
    "Salir",

    "PAUSA",
    "Música",
    "SFX",
    "Idioma",
    "Continuar",
    "Volver al menú",

    "GAME OVER",
    "Récord",

    "TETRIS",
    "Puntuación:",
    "Líneas:",
    "Nivel:",
    "Siguiente:",
};

// --- Portugais ---------------------------------------------------------------
static const char* PT[] = {
    "Jogar",
    "Opções",
    "Sair",

    "PAUSA",
    "Música",
    "SFX",
    "Idioma",
    "Continuar",
    "Menu",

    "GAME OVER",
    "Recorde",

    "TETRIS",
    "Pontuação:",
    "Linhas:",
    "Nível:",
    "Seguinte:",
};

// --- Danois ------------------------------------------------------------------
static const char* DK[] = {
    "Spil",
    "Indstillinger",
    "Afslut",

    "PAUSE",
    "Musik",
    "SFX",
    "Sprog",
    "Fortsæt",
    "Til menu",

    "GAME OVER",
    "Rekord",

    "TETRIS",
    "Point:",
    "Linjer:",
    "Niveau:",
    "Næste:",
};

// --- Néerlandais -------------------------------------------------------------
static const char* NL[] = {
    "Spelen",
    "Opties",
    "Afsluiten",

    "PAUZE",
    "Muziek",
    "SFX",
    "Taal",
    "Hervatten",
    "Terug naar menu",

    "GAME OVER",
    "Record",

    "TETRIS",
    "Score:",
    "Lijnen:",
    "Niveau:",
    "Volgende:",
};

// -----------------------------------------------------------------------------
//  Sélecteur de table
// -----------------------------------------------------------------------------
static const char** get_table(Language lang)
{
    switch (lang)
    {
        case Language::FR: return FR;
        case Language::EN: return EN;
        case Language::DE: return DE;
        case Language::IT: return IT;
        case Language::ES: return ES;
        case Language::PT: return PT;
        case Language::DK: return DK;
        case Language::NL: return NL;

        case Language::COUNT:
            break; // valeur sentinelle, jamais utilisée
    }

    return EN; // fallback
}

// -----------------------------------------------------------------------------
//  API principale
// -----------------------------------------------------------------------------

void language_set(Language lang)
{
    if ((int)lang < 0 || (int)lang >= (int)Language::COUNT)
        lang = Language::EN;

    g_lang = lang;
    language_save_to_nvs();
}

Language language_get()
{
    return g_lang;
}

void language_next()
{
    int v = (int)g_lang;
    v = (v + 1) % (int)Language::COUNT;
    language_set((Language)v);
}

void language_prev()
{
    int v = (int)g_lang;
    v = (v - 1 + (int)Language::COUNT) % (int)Language::COUNT;
    language_set((Language)v);
}

const char* tr(TextID id)
{
    return get_table(g_lang)[(int)id];
}

const char* language_name(Language lang)
{
    switch (lang)
    {
        case Language::FR: return "Français";
        case Language::EN: return "English";
        case Language::DE: return "Deutsch";
        case Language::IT: return "Italiano";
        case Language::ES: return "Español";
        case Language::PT: return "Português";
        case Language::DK: return "Dansk";
        case Language::NL: return "Nederlands";

        case Language::COUNT:
            break;
    }

    return "???";
}

// -----------------------------------------------------------------------------
//  Persistance NVS
// -----------------------------------------------------------------------------

void language_load_from_nvs()
{
    g_lang = Language::EN;

    nvs_handle_t h;
    if (nvs_open("settings", NVS_READONLY, &h) == ESP_OK)
    {
        uint8_t v = (uint8_t)Language::EN;

        if (nvs_get_u8(h, "lang", &v) == ESP_OK)
        {
            if (v < (uint8_t)Language::COUNT)
                g_lang = (Language)v;
        }

        nvs_close(h);
    }
}

void language_save_to_nvs()
{
    nvs_handle_t h;
    if (nvs_open("settings", NVS_READWRITE, &h) == ESP_OK)
    {
        nvs_set_u8(h, "lang", (uint8_t)g_lang);
        nvs_commit(h);
        nvs_close(h);
    }
}
