#include "Frontend/Renderer.h"
#include "event_filter_function.h"
#include "../tests/test.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <SDL.h>

int main(int argc, char** argv)
{
	Renderer_Init();

	//Run_All_Tests();

	Bus bus;
	FILE* file = fopen("tests/stack_test.bin", "r");
	fread(bus.memory, 1, 64 * 1024, file);
	fclose(file);

	State6502 cpu;
	cpu.bus = &bus;
	power_on(&cpu);
	reset(&cpu);

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
}
