#include "Frontend/Renderer.h"
#include "event_filter_function.h"
#include "Backend/Cartridge.h"
#include "Backend/Mappers/Mapper_000.h"
#include "test.h"

#include "timer.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <SDL.h>

int main(int argc, char** argv)
{
	RendererInit();

	Run_All_Tests();

	Cartridge cart;
	//load_cartridge_from_file(&cart, "tests/roms/clear_color_test.nes");
	//load_cartridge_from_file(&cart, "tests/roms/4.vbl_clear_timing.nes"); // failed
	//load_cartridge_from_file(&cart, "tests/roms/1.frame_basics.nes");
	//load_cartridge_from_file(&cart, "tests/roms/nmi_sync.nes");
	//load_cartridge_from_file(&cart, "tests/roms/palette.nes");
	load_cartridge_from_file(&cart, "tests/roms/nestest.nes");
	//load_cartridge_from_file(&cart, "tests/roms/scanline.nes");

	//load_cartridge_from_file(&cart, "roms/DonkeyKong.nes");
	//load_cartridge_from_file(&cart, "roms/SuperMarioBros.nes");

	// TODO: put this into a function
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

	long long clocks = 0;

	RendererSetPaletteData(ppu_bus.palette);

	uint8_t* chr = ((Mapper000*)(cart.mapper))->CHR;

	RendererDraw(&cpu, &ppu);

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

					RendererDraw(&cpu, &ppu);
					break;
				case SDLK_f:
				{
					LoadPatternTable(chr, 0, 0);
					LoadPatternTable(chr + 0x1000, 1, 0);
					do
					{
						clocks++;
						clock_2C02(&ppu);
						if (clocks % 3 == 0)
							clock_6502(&cpu);
					} while (!(ppu.scanline == 241 && ppu.cycles == 0));
					RendererDraw(&cpu, &ppu);
					break;
				}
				case SDLK_p:
					clocks++;
					clock_2C02(&ppu);
					if (clocks % 3 == 0)
						clock_6502(&cpu);
					RendererDraw(&cpu, &ppu);
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

	RendererShutdown();
	return 0;

#if 0
	RendererInit();

	Run_All_Tests();
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
