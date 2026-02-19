	#include "audio_pmf.h"
	#include "audio.h"
	#include "gb_audio_player.h"
	#include "gb_audio_track_pmf.h"
	#include "gb_audio_track_tone.h"
	#include "gb_audio_track_wav.h"

	// -----------------------------------------------------------------------------
	// Données PMF
	// -----------------------------------------------------------------------------

	#include "../assets/pmf/Tetris_title.h"
	#include "../assets/pmf/Tetris_gameover.h"
	#include "../assets/pmf/Tetris_hiscore.h"
	#include "../assets/pmf/Tetris_1.h"
	#include "../assets/pmf/Tetris_2.h"
	#include "../assets/pmf/Tetris_3.h"
	#include "../assets/pmf/Tetris_4.h"

	// -----------------------------------------------------------------------------
	// Player & pistes
	// -----------------------------------------------------------------------------

	extern gb_audio_player     g_player;
	extern gb_audio_track_pmf  g_track_music;

	static MusicID        g_current_music      = MusicID::NONE;
	static uint8_t        g_current_level      = 1;
	static const uint8_t* g_current_game_pmf   = nullptr;

	// -----------------------------------------------------------------------------
	// Sélection musique in‑game selon niveau
	// -----------------------------------------------------------------------------

	static const uint8_t* select_ingame_pmf_for_level(uint8_t level)
	{
		if (level <= 5)   return Tetris_1_pmf;
		if (level <= 15)  return Tetris_2_pmf;
		if (level <= 25)  return Tetris_3_pmf;
		if (level <= 35)  return Tetris_4_pmf;
		if (level <= 50)  return Tetris_1_pmf;
		return Tetris_2_pmf;
	}

	// -----------------------------------------------------------------------------
	// Mapping MusicID → PMF
	// -----------------------------------------------------------------------------

	static const uint8_t* get_music_data(MusicID id)
	{
		switch (id)
		{
			case MusicID::MENU:     return Tetris_title_pmf;
			case MusicID::GAME:     return select_ingame_pmf_for_level(g_current_level);
			case MusicID::GAMEOVER: return Tetris_gameover_pmf;
			case MusicID::HISCORE:  return Tetris_hiscore_pmf;
			default:                return nullptr;
		}
	}

	// -----------------------------------------------------------------------------
	// API
	// -----------------------------------------------------------------------------

	void audio_pmf_init()
	{
		// Volume maître par défaut
		g_player.set_master_volume(180);

		g_current_music    = MusicID::NONE;
		g_current_level    = 1;
		g_current_game_pmf = nullptr;
	}

	void audio_pmf_update()
	{
		// Fait avancer la musique + SFX et pousse dans l’I2S
		g_player.pool();
	}

	void audio_pmf_play(MusicID id, bool loop)
	{
		(void)loop; // PMF gère lui-même la boucle

		const uint8_t* data = get_music_data(id);
		if (!data)
			return;

		g_track_music.stop_playing();
		g_track_music.load_pmf(data);
		g_track_music.play_pmf();

		g_current_music = id;

		if (id == MusicID::GAME)
			g_current_game_pmf = data;
		else
			g_current_game_pmf = nullptr;
	}

	void audio_pmf_stop()
	{
		g_track_music.stop_playing();
		g_current_music    = MusicID::NONE;
		g_current_game_pmf = nullptr;
	}

	void audio_pmf_set_level(uint8_t level)
	{
		if (level == 0)
			level = 1;

		g_current_level = level;

		if (g_current_music == MusicID::GAME)
		{
			const uint8_t* new_data = select_ingame_pmf_for_level(g_current_level);

			if (new_data != g_current_game_pmf)
			{
				g_track_music.stop_playing();
				g_track_music.load_pmf(new_data);
				g_track_music.play_pmf();

				g_current_game_pmf = new_data;
			}
		}
	}

	void audio_pmf_set_master_volume(uint8_t v)
	{
		g_player.set_master_volume(v);
	}
