#include "Frontend/Renderer.h"
#include "event_filter_function.h"
#include "Backend/Cartridge.h"
#include "Backend/Mappers/Mapper_000.h"

#include "timer.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <SDL.h>

int main(int argc, char** argv)
{
	Renderer_Init();

	Cartridge cart;
	load_cartridge_from_file(&cart, "tests/full_nes_palette.nes");
	//load_cartridge_from_file(&cart, "roms/DonkeyKong.nes");

	Bus6502 cpu_bus;
	Bus2C02 ppu_bus;
	State6502 cpu;
	State2C02 ppu;

	cpu_bus.cartridge = &cart;
	cpu_bus.ppu = &ppu;

	ppu_bus.cartridge = &cart;
	
	cpu.bus = &cpu_bus;

	ppu.bus = &ppu_bus;
	ppu.cpu = &cpu;

	power_on_6502(&cpu);
	reset_6502(&cpu);

	power_on_2C02(&ppu);
	reset_2C02(&ppu);

	long clocks = 0;
	Renderer_Draw(&cpu, &ppu);

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
					while (true)
					{
						clocks++;
						clock_2C02(&ppu);
						if (clocks % 3 == 0 && clock_6502(&cpu) == 0)
							break;
					}

					Renderer_Draw(&cpu, &ppu);
					break;
				case SDLK_f:
				{
					do
					{
						clocks++;
						clock_2C02(&ppu);
						if (clocks % 3 == 0)
							clock_6502(&cpu);
					} while (!(ppu.scanline == 241 && ppu.cycles == 0));
					Renderer_Draw(&cpu, &ppu);
					break;
				}
				case SDLK_p:
					clocks++;
					clock_2C02(&ppu);
					if (clocks % 3 == 0)
						clock_6502(&cpu);
					Renderer_Draw(&cpu, &ppu);
					break;
				}
			}
			if (event.type == SDL_QUIT)
			{
				exit(EXIT_SUCCESS);
			}
		}
	}

	fgetc(stdin);

	free_cartridge(&cart);

	Renderer_Shutdown();
	return 0;

#if 0
	Renderer_Init();

	Run_All_Tests();
	RunAllBenchmarks();
	Bus6502 bus;
	State6502 cpu;
	load_cpu_from_file(&cpu, &bus, "tests/stack_test.bin");

	Renderer_Draw(&cpu);

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
					Renderer_Draw(&cpu);
					break;
				case SDLK_LEFT:
					Renderer_SetPageView(--page);
					Renderer_Draw(&cpu);
					break;
				case SDLK_RIGHT:
					Renderer_SetPageView(++page);
					Renderer_Draw(&cpu);
					break;
				case SDLK_UP:
					page = (int)page - 16;
					Renderer_SetPageView(page);
					Renderer_Draw(&cpu);
					break;
				case SDLK_DOWN:
					page += 16;
					Renderer_SetPageView(page);
					Renderer_Draw(&cpu);
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
	Renderer_Shutdown();
	return 0;

#endif
}
