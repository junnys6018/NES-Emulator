#include "Frontend/Renderer.h"
#include "event_filter_function.h"
#include "../tests/test.h"
#include "../tests/Benchmarks.h"
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

	//Run_All_Tests();
	//RunAllBenchmarks();

	Cartridge cart;
	//load_cartridge_from_file(&cart, "tests/nestest.nes");
	load_cartridge_from_file(&cart, "roms/SuperMarioBros.nes");

	uint8_t* chr = ((Mapper000*)(cart.mapper))->CHR;

	uint8_t palette[4] = { 0x0F,0x16,0x1A,0x01 }; // Black, red, green, blue

	timepoint beg, end;

	GetTime(&beg);
	LoadPatternTable(chr, 0, palette);
	LoadPatternTable(chr + 0x1000, 1, palette);
	GetTime(&end);

	printf("time: %.3fms", GetElapsedTimeMicro(&beg, &end) / 1000);

	DrawPatternTable(0, 0, 0);
	DrawPatternTable(260, 0, 1);
	//Bus6502 bus;
	//State6502 cpu;
	//load_cpu_from_file(&cpu, &bus, "tests/stack_test.bin");

	//Renderer_Draw(&cpu);

	//EventTypeList list = { .size = 2,.event_types = {SDL_KEYDOWN, SDL_QUIT} };
	//SDL_SetEventFilter(event_whitelist, &list);
	//uint8_t page = 0;
	//SDL_Event event;
	//while (true)
	//{
	//	while (SDL_PollEvent(&event) != 0)
	//	{
	//		if (event.type == SDL_KEYDOWN)
	//		{
	//			switch (event.key.keysym.sym)
	//			{
	//			case SDLK_SPACE:
	//				while(clock_6502(&cpu) != 0);
	//				Renderer_Draw(&cpu);
	//				break;
	//			case SDLK_LEFT:
	//				Renderer_SetPageView(--page);
	//				Renderer_Draw(&cpu);
	//				break;
	//			case SDLK_RIGHT:
	//				Renderer_SetPageView(++page);
	//				Renderer_Draw(&cpu);
	//				break;
	//			case SDLK_UP:
	//				page = (int)page - 16;
	//				Renderer_SetPageView(page);
	//				Renderer_Draw(&cpu);
	//				break;
	//			case SDLK_DOWN:
	//				page += 16;
	//				Renderer_SetPageView(page);
	//				Renderer_Draw(&cpu);
	//				break;
	//			}
	//		}
	//		if (event.type == SDL_QUIT)
	//		{
	//			exit(EXIT_SUCCESS);
	//		}
	//	}
	//}

	//SDL_SetEventFilter(reset_filter_event, NULL);

	fgetc(stdin);

	free_cartridge(&cart);

	Renderer_Shutdown();
	return 0;
}
