| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- | -------- | -------- |

# TAKATRIS – version Gamebuino AKA
Portage du jeu Tetris pour plateforme Gamebuino AKA / ESP32. 
Projet structuré pour l’ESP-IDF avec CMake, organisé en modules et ressources audio. Réalisé en février 2026. Done by Jicehel. [github.com]

Repo : https://github.com/jeancharleslebeau-gif/Tetris---version-Gamebuino-AKA


✨ Caractéristiques

Cibles matérielles ESP32 : ESP32, ESP32‑C2/C3/C5/C6, ESP32‑H2/P4, ESP32‑S2/S3. [github.com]
Langages : C (88.9%), C++ (11%). [github.com]
Structure propre en modules ESP‑IDF (components/) et application (main/).
Ressources audio PMF intégrées dans Assets/pmf/.

🗂️ Arborescence

```
Tetris---version-Gamebuino-AKA/
├── components/
├── main/
├── Assets/
│   └── pmf/
│       ├── Tetris_1.h
│       ├── Tetris_2.h
│       ├── Tetris_3.h
│       ├── Tetris_4.h
│       ├── Tetris_gameover.h
│       ├── Tetris_hiscore.h
│       └── Tetris_title.h
├── CMakeLists.txt
├── partitions.csv
├── sdkconfig
└── README.md
```

🧭 Architecture logicielle

- main/
Point d’entrée du jeu, initialisation matériel, boucle principale.
- components/
Modules ESP‑IDF (rendu, logique, entrée utilisateur…).
(sources de JMP42: https://github.com/jmp42/Gamebuino_AKA_lib)
- Assets/pmf/
Musiques du jeu intégrées en binaire.

Assets & Audio PMF
🎨 Assets
Le répertoire Assets/ contient les ressources multimédia utilisées par le jeu.
Dans ce projet, il ne contient que les musiques, organisées dans un sous‑répertoire dédié :
Assets/
└── pmf/

🎵 pmf/ — Musiques du jeu au format PMF
Le dossier Assets/pmf/ regroupe l'ensemble des thèmes musicaux du jeu, tous fournis sous forme de fichiers .h générés à partir de fichiers audio modulaires (MOD, XM, S3M, IT…) convertis au format PMF puis exportés en header C pour être intégrés dans le fichier binaire du jeu.

📦 Contenu du dossier Assets/pmf/
Fichier .h        Rôle dans le jeu
Tetris_1.h        Thème de niveau (musique in‑game)
Tetris_2.h        Thème de niveau (musique in‑game)
Tetris_3.h        Thème de niveau (musique in‑game)
Tetris_4.h        Thème de niveau (musique in‑game)
Tetris_title.h    Musique de l’écran titre
Tetris_gameover.h Musique de Game Over
Tetris_hiscore.h  Musique de l’écran High Score

🛠️ Le format PMF (Portable Music Format)

PMF est un format audio optimisé pour les systèmes embarqués (dont ESP32).
Il permet de jouer des musiques riches (proches de MOD/XM) mais avec une empreinte mémoire réduite.
Les fichiers .pmf ont été convertis en .h, ce qui permet :
- une inclusion directe dans le firmware
- une lecture sans avoir à accéder au système de fichiers
- une décompression/lecture streaming efficace







