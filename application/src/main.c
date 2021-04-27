#include "Application.h"
#include "Gui.h"
#include "Audio.h"
#include "StartupOptions.h"
#include "Models/SettingsModel.h"

#include "nes.h"

#include "util/timer.h"

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
	InitSDLAudio();

	//if (argc == 2 && strcmp(argv[1], "--test") == 0)
	//{
	//	RunAll6502Tests();
	//	RunAll2C02Tests();
	//}
	//else if (argc == 2 && strcmp(argv[1], "--benchmark") == 0)
	//{
	//	RunAllBenchmarks();
	//}

	InitApplication(NULL);
	ApplicationGameLoop();
	ShutdownApplication();


	ShutdownSDLAudio();
	SDL_Quit();

	exit(EXIT_SUCCESS);
}
