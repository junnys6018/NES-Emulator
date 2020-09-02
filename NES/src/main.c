#include "Frontend/Renderer.h"

#include "../tests/test.h"
#include <stdio.h>
#include <SDL.h>
#include <stdbool.h>

int main(int argc, char** argv)
{
	Renderer_Init();

	Run_All_Tests();

	//Bus bus;
	//FILE* file = fopen("tests/6502_functional_test.bin", "r");
	//fread(bus.memory, 1, 64 * 1024, file);
	//fclose(file);

	//State6502 cpu;
	//cpu.bus = &bus;
	//power_on(&cpu);
	//reset(&cpu);

	//cpu.PC = 0x0400;

	////for (int i = 0; i < 84030250; i++)
	////{
	////	clock_6502(&cpu);
	////}

	//Renderer_Draw(&cpu);

	//SDL_Event event;
	//bool quit = false;
	//while (!quit)
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
	//			}
	//		}
	//		if (event.type == SDL_QUIT)
	//		{
	//			quit = true;
	//		}
	//	}
	//}

	fgetc(stdin);
	Renderer_Shutdown();
	return 0;
}
