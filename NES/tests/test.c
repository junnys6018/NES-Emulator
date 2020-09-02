#include "test.h"

#include "Frontend/Renderer.h"
#include "Backend/6502.h"
#include "event_filter_function.h"

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

void Run_6502_Functional_Test()
{
	Bus bus;
	FILE* file = fopen("tests/6502_functional_test.bin", "rb");
	fread(bus.memory, 1, 64 * 1024, file);
	fclose(file);

	State6502 cpu;
	cpu.bus = &bus;
	power_on(&cpu);
	reset(&cpu);

	cpu.PC = 0x0400;

	SDL_AddTimer(16, on_render_callback, NULL);

	EventTypeList list = { .size = 2,.event_types = {SDL_USEREVENT, SDL_QUIT} };
	SDL_SetEventFilter(event_whitelist, &list);

	uint16_t old_PC = 0x400;
	SDL_Event event;
	Renderer_Draw(&cpu);
	while (true)
	{
		for (int i = 0; i < 5000; i++)
		{
			clock_6502(&cpu);
		}

		// Poll Events
		while (SDL_PollEvent(&event) != 0)
		{
			if (event.type == SDL_QUIT)
			{
				exit(EXIT_SUCCESS);
			}
			else if (event.type == SDL_USEREVENT && event.user.code == 0)
			{
				Renderer_Draw(&cpu);
			}
		}

		// Check for success or failure
		if (old_PC == cpu.PC)
		{
			uint8_t success_opcode_pattern[] = { 0x4C, 0xBA, 0xEA, 0xBA, 0xEA };
			uint8_t opcode_size[] = { 3, 1, 1, 1, 1 };
			bool passed = true;
			for (int i = 0; i < 5; i++)
			{
				uint8_t opcode = bus_read(cpu.bus, old_PC);
				passed = passed && (opcode == success_opcode_pattern[i]);
				old_PC += opcode_size[i];
			}

			if (passed)
				printf("Passed\n");
			else
				printf("Failed\n");

			Renderer_Draw(&cpu);
			break;
		}
		old_PC = cpu.PC;
	}


	SDL_SetEventFilter(reset_filter_event, NULL);
}


void Run_All_Tests()
{
	Run_6502_Functional_Test();
}
