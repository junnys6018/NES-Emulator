#include "Frontend/Renderer.h"
#include "Frontend/Gui.h"
#include "Frontend/Controller.h"

#include "event_filter_function.h"

#include "Backend/Mappers/Mapper_000.h"
#include "Backend/nes.h"

#include "test_6502.h"
#include "test_2C02.h"
#include "Benchmarks.h"

#include "timer.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <SDL.h>

int main(int argc, char** argv)
{
	Controller controller = { .mode = MODE_PLAY };
	RendererInit(&controller);

	//RunAll6502Tests();
	//RunAll2C02Tests();
	//RunAllBenchmarks();

	Nes nes;
	//NESInit(&nes, "roms/SuperMarioBros.nes");
	//NESInit(&nes, "roms/DonkeyKong.nes");
	//NESInit(&nes, "tests/roms/nestest.nes");
	//NESInit(&nes, "tests/roms/clear_color_test.nes");
	NESInit(&nes, "tests/roms/palette.nes");
	RendererBindNES(&nes);

	// TODO: 
	uint8_t* chr = ((Mapper000*)(nes.cart.mapper))->CHR;
	RendererSetPatternTable(chr, 0);
	RendererSetPatternTable(chr + 0x1000, 1);

	SDL_Event event;
	timepoint beg, end;
	while (true)
	{
		GetTime(&beg);
		poll_keys(&nes.pad);

		if (controller.mode == MODE_PLAY)
		{
			clock_nes_frame(&nes);
		}

		while (SDL_PollEvent(&event) != 0)
		{
			GuiDispatchEvent(&event);
			if (controller.mode == MODE_STEP_THROUGH && event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_SPACE:
					clock_nes_instruction(&nes);
					break;
				case SDLK_f:
				{
					clock_nes_frame(&nes);
					break;
				}
				case SDLK_p:
					clock_nes_cycle(&nes);
					break;
				}
			}
			if (event.type == SDL_QUIT)
			{
				exit(EXIT_SUCCESS);
			}
		}

		RendererDraw();
		GetTime(&end);
		float time = 16666 - GetElapsedTimeMicro(&beg, &end);
		if (time > 0)
		{
			SleepMicro((uint64_t)time);
		}
	}

	NESDestroy(&nes);
	RendererShutdown();

	exit(EXIT_SUCCESS);
}
