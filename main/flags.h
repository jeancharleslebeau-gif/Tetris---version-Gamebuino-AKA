/*
===============================================================================
  flags.h — Atlas des drapeaux (Tetris AKA)
-------------------------------------------------------------------------------
  Rôle :
    - Déclarer l’atlas contenant les drapeaux des langues.
    - Fournir les dimensions standard d’un drapeau.
    - Fournir l’accès au tableau binaire (défini dans flags.cpp).

  Auteur : Jean‑Charles LEBEAU (Jicehel)
  Création : 02/2026
===============================================================================
*/

#pragma once

#include <stdint.h>

// -----------------------------------------------------------------------------
//  Dimensions d’un drapeau dans l’atlas
// -----------------------------------------------------------------------------
static constexpr int FLAG_WIDTH  = 32;   // largeur d’un drapeau
static constexpr int FLAG_HEIGHT = 20;   // hauteur d’un drapeau

// Nombre total de drapeaux dans l’atlas
static constexpr int FLAG_COUNT = 8;

// -----------------------------------------------------------------------------
//  Données de l’atlas (définies dans flags.cpp)
// -----------------------------------------------------------------------------

// Tableau contenant l’image complète (format RGB565 ou autre selon ton moteur)
extern const uint16_t flags_atlas[];

// Largeur totale de l’atlas (FLAG_WIDTH * FLAG_COUNT)
static constexpr int FLAGS_ATLAS_WIDTH  = FLAG_WIDTH * FLAG_COUNT;

// Hauteur totale de l’atlas
static constexpr int FLAGS_ATLAS_HEIGHT = FLAG_HEIGHT;
