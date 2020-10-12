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
#include "FileDialog.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <SDL.h>

int main(int argc, char** argv)
{
	Controller controller = { .mode = MODE_NOT_RUNNING };
	RendererInit(&controller);

	//RunAll6502Tests();
	//RunAll2C02Tests();
	//RunAllBenchmarks();

	Nes nes;
	RendererBindNES(&nes);
	NESInit(&nes, NULL);

	//NESInit(&nes, "roms/SuperMarioBros.nes");
	//NESInit(&nes, "roms/DonkeyKong.nes");
	//NESInit(&nes, "roms/Tetris.nes");
	//NESInit(&nes, "roms/LegendofZelda.nes");
	//NESInit(&nes, "roms/PacMan.nes");
	//NESInit(&nes, "roms/BalloonFight.nes");
	//NESInit(&nes, "roms/MicroMages.nes");

	//NESInit(&nes, "tests/roms/nestest.nes");
	//NESInit(&nes, "tests/roms/full_nes_palette.nes");
	//NESInit(&nes, "tests/roms/nmi_sync.nes");
	//NESInit(&nes, "tests/roms/blargg_tests/sprite_hit_tests_2005.10.05/01.basics.nes");
	//NESInit(&nes, "tests/roms/ppu_read_buffer/test_ppu_read_buffer.nes");

	//nes.pad.current_input.reg = 0x00;
	//for (int i = 0; i < 11199700; i++)
	//{
	//	clock_nes_cycle(&nes);
	//	clock_nes_cycle(&nes);
	//	clock_nes_cycle(&nes);
	//}

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
					clock_nes_frame(&nes);
					break;
				case SDLK_p:
					clock_nes_cycle(&nes);
					break;
				}
			}
			else if (event.type == SDL_QUIT)
			{
				exit(EXIT_SUCCESS);
			}
		}

		RendererDraw();
		GetTime(&end);
		printf("Took %.3fms                   \r", GetElapsedTimeMilli(&beg, &end));
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
