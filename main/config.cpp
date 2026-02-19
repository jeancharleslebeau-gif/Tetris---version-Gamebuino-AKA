/*
===============================================================================
  config.cpp — Configuration globale du moteur
-------------------------------------------------------------------------------
  Rôle :
    - Définir les variables globales déclarées dans config.h
    - Fournir load_config() / save_config() si nécessaire
===============================================================================
*/

#include <cstdint>
#include "config.h"

// -----------------------------------------------------------------------------
// Variables globales
// -----------------------------------------------------------------------------

// Rotation écran (false = paysage, true = portrait)
bool g_rotate_screen = false;

// Mode debug
int debug = 0;

// Définition des volumes sonores par défaut
uint8_t volume_music = 7;
uint8_t volume_sfx   = 6;


// -----------------------------------------------------------------------------
// Configuration persistante (optionnel)
// Pour l’instant, on laisse vide. Tu pourras ajouter la NVS ici plus tard.
// -----------------------------------------------------------------------------

void load_config()
{
    // Exemple futur :
    // g_rotate_screen = load_from_nvs("rotate", false);
}

void save_config()
{
    // Exemple futur :
    // save_to_nvs("rotate", g_rotate_screen);
}
