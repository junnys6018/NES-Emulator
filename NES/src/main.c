#include <SDL.h>

#include "Frontend/Renderer.h"
#include "6502.h"

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

int main()
{
	Renderer_Init();

	Bus bus;
	FILE* file = fopen("6502_functional_test.bin", "r");
	fread(bus.memory, 1, 64 * 1024, file);
	fclose(file);

	State6502 cpu;
	cpu.bus = &bus;
	power_on(&cpu);
	reset(&cpu);

	cpu.PC = 0x0400;

	//for (int i = 0; i < 84000000 + 30250; i++)
	//{
	//	clock(&cpu);
	//}

	Renderer_Draw(&cpu);

	SDL_Event event;
	bool quit = false;
	while (!quit)
	{
		while (SDL_PollEvent(&event) != 0)
		{
			if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_SPACE:
					while (clock(&cpu) != 0);
					Renderer_Draw(&cpu);
					break;
				}
			}
			if (event.type == SDL_QUIT)
			{
				quit = true;
			}
		}
	}

	Renderer_Shutdown();
	return 0;
}
