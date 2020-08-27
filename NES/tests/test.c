#include "test.h"
#include "Backend/6502.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include <SDL.h>

Uint32 on_render_callback(Uint32 interval, void* param)
{
	SDL_Event event;
	SDL_UserEvent userevent;

	userevent.type = SDL_USEREVENT;
	userevent.code = 0;
	userevent.data1 = NULL;
	userevent.data2 = NULL;

	event.type = SDL_USEREVENT;
	event.user = userevent;

	SDL_PushEvent(&event);

	return interval;
}

int on_filter_event(void* userdata, SDL_Event* event)
{
	return (event->type == SDL_QUIT || (event->type == SDL_USEREVENT && event->user.code == 0));
}

void Run_6502_Functional_Test()
{
	Bus bus;
	FILE* file = fopen("tests/6502_functional_test.bin", "r");
	fread(bus.memory, 1, 64 * 1024, file);
	fclose(file);

	State6502 cpu;
	cpu.bus = &bus;
	power_on(&cpu);
	reset(&cpu);

	cpu.PC = 0x0400;

	//for (int i = 0; i < 84030250; i++)
	//{
	//	clock(&cpu);
	//}

	SDL_AddTimer(16, on_render_callback, NULL);

	Renderer_Draw(&cpu);

	SDL_Event event;
	bool done = false;
	while (!done)
	{
		clock_6502(&cpu);

		// Poll Events
		SDL_FilterEvents(on_filter_event, NULL);
		while (SDL_PollEvent(&event) != 0)
		{
			if (event.type == SDL_QUIT)
			{
				exit(EXIT_SUCCESS);
			}
			else if (event.type == SDL_USEREVENT && event.user.code == 0)
			{
				clock_t start, end;
				double cpu_time_used;

				start = clock();
				Renderer_Draw(&cpu);
				end = clock();
				cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

				printf("took %f ms\n", cpu_time_used * 1000.0f);
			}
		}

	}
}


void Run_All_Tests()
{
	Run_6502_Functional_Test();
}
