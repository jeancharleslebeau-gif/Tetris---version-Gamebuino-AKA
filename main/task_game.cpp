	/*
	===============================================================================
	  task_game.cpp — Boucle de jeu (AKA Edition)
	===============================================================================
	*/

	#include "freertos/FreeRTOS.h"
	#include "freertos/task.h"

	#include "task_game.h"
	#include "game.h"
	#include "graphics.h"
	#include "input.h"
	#include "gb_core.h"

	extern gb_core g_core;

	void task_game(void*)
	{
		const TickType_t updateDelay = pdMS_TO_TICKS(16); // ~60 FPS
		const TickType_t inputDelay  = pdMS_TO_TICKS(4);  // 250 Hz input

		TickType_t lastUpdate = xTaskGetTickCount();
		TickType_t lastInput  = xTaskGetTickCount();

		game_init();

		while(true)
		{
			TickType_t now = xTaskGetTickCount();

			// --- INPUT : haute fréquence ---
			if(now - lastInput >= inputDelay)
			{
				g_core.pool();
				input_poll();
				lastInput = now;
			}

			// --- UPDATE + RENDER : 60 FPS ---
			if(now - lastUpdate >= updateDelay)
			{
				game_update();
				game_render();
				lastUpdate = now;
			}

			vTaskDelay(1); // yield propre
		}
	}
