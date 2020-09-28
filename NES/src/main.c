#include "Frontend/Renderer.h"
#include "event_filter_function.h"
#include "Backend/Mappers/Mapper_000.h"
#include "Backend/nes.h"

#include "test.h"
#include "Benchmarks.h"

#include "timer.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <SDL.h>

int main(int argc, char** argv)
{
	RendererInit();

	RunAllTests();
	RunAllBenchmarks();
	
	Nes nes;
	
	//NESInit(&nes, "roms/SuperMarioBros.nes");
	NESInit(&nes, "tests/roms/nestest.nes");

	RendererBindNES(&nes);

	uint8_t* chr = ((Mapper000*)(nes.cart.mapper))->CHR;
	RendererSetPatternTable(chr, 0);
	RendererSetPatternTable(chr + 0x1000, 1);

	RendererDraw();

	EventTypeList list = { .size = 2,.event_types = {SDL_KEYDOWN, SDL_QUIT} };
	SDL_SetEventFilter(event_whitelist, &list);

	SDL_Event event;
	while (true)
	{
		while (SDL_PollEvent(&event) != 0)
		{
			if (event.type == SDL_KEYDOWN)
			{
				RendererDraw();
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
	}

	NESDestroy(&nes);

	SDL_SetEventFilter(reset_filter_event, NULL);

	RendererShutdown();
	return 0;

#if 0
	RendererInit();

	RunAllTests();
	RunAllBenchmarks();
	Bus6502 bus;
	State6502 cpu;
	load_cpu_from_file(&cpu, &bus, "tests/stack_test.bin");

	RendererDraw(&cpu);

	EventTypeList list = { .size = 2,.event_types = {SDL_KEYDOWN, SDL_QUIT} };
	SDL_SetEventFilter(event_whitelist, &list);
	uint8_t page = 0;
	SDL_Event event;
	while (true)
	{
		while (SDL_PollEvent(&event) != 0)
		{
			if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_SPACE:
					while(clock_6502(&cpu) != 0);
					RendererDraw(&cpu);
					break;
				case SDLK_LEFT:
					RendererSetPageView(--page);
					RendererDraw(&cpu);
					break;
				case SDLK_RIGHT:
					RendererSetPageView(++page);
					RendererDraw(&cpu);
					break;
				case SDLK_UP:
					page = (int)page - 16;
					RendererSetPageView(page);
					RendererDraw(&cpu);
					break;
				case SDLK_DOWN:
					page += 16;
					RendererSetPageView(page);
					RendererDraw(&cpu);
					break;
				}
			}
			if (event.type == SDL_QUIT)
			{
				exit(EXIT_SUCCESS);
			}
		}
	}

	SDL_SetEventFilter(reset_filter_event, NULL);

	fgetc(stdin);
	RendererShutdown();
	return 0;

#endif
}
