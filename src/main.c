#include "Frontend/Controller.h"
#include "Frontend/Gui.h"
#include "Frontend/Audio.h"
#include "Frontend/StartupOptions.h"
#include "Frontend/Models/SettingsModel.h"

#include "Backend/nes.h"

#include "test_6502.h"
#include "test_2C02.h"
#include "Benchmarks.h"
#include "timer.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <SDL.h>

int main(int argc, char** argv)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("[ERROR] Could not initialize SDL");
		exit(EXIT_FAILURE);
	}

	// Initialize all the things
	LoadStartupOptions();
	AudioPrecompute();
	InitController();
	InitSDLAudio();

	SettingsModel* settings = GetSettings();

	Nes nes;
	InitNES(&nes, NULL);
	if (argc == 2 && strcmp(argv[1], "--test") == 0)
	{
		RunAll6502Tests();
		RunAll2C02Tests();
	}
	else if (argc == 2 && strcmp(argv[1], "--benchmark") == 0)
	{
		RunAllBenchmarks();
	}
	else if (argc == 2)
	{
		NESDestroy(&nes);
		if (InitNES(&nes, argv[1]) == 0)
		{
			settings->mode = MODE_PLAY;
		}
		else
		{
			printf("[ERROR] Failed to load %s as an ines rom\n", argv[1]);
			InitNES(&nes, NULL);
		}
	}

	ControllerBindNES(&nes);

	int window = 10;
	float total_time = 0.0f;
	int curr_frame = 0;

	SDL_Event event;
	timepoint beg, end;
	bool running = true;
	while (running)
	{
		GetTime(&beg);
		poll_keys(&nes.pad);

		if (settings->mode == MODE_PLAY)
		{
			clock_nes_frame(&nes);
			if (nes.apu.audio_pos != 0)
			{
				WriteSamples(nes.apu.audio_buffer, nes.apu.audio_pos);
				nes.apu.audio_pos = 0;
			}
		}

		ControllerDrawViews();

		while (SDL_PollEvent(&event) != 0)
		{
			GuiDispatchEvent(&event);
			if (settings->mode == MODE_STEP_THROUGH && event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_SPACE:
					clock_nes_instruction(&nes);
					break;
				case SDLK_f:
					clock_nes_frame(&nes);
					break;
				case SDLK_p:
					clock_nes_cycle(&nes);
					break;
				}
			}
			else if (event.type == SDL_QUIT)
			{
				running = false;
			}
		}

		GetTime(&end);
		total_time += GetElapsedTimeMilli(&beg, &end);
		curr_frame++;
		if (curr_frame == window)
		{
			settings->ms_per_frame = total_time / window;

			total_time = 0.0f;
			curr_frame = 0;
		}

		float elapsed = GetElapsedTimeMicro(&beg, &end);
		if (elapsed < 16666) // 60 FPS
		{
			SleepMicro((uint64_t)(16666 - elapsed));
		}
	}

	NESDestroy(&nes);
	ControllerShutdown();
	ShutdownSDLAudio();
	SDL_Quit();

	exit(EXIT_SUCCESS);
}
