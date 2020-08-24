#include <SDL.h> 
#include <stdio.h>

#include "6502.h"

int width = 1080, height = 720;
void DrawScreen(SDL_Renderer* rend)
{
	// Clear screen to black
	SDL_SetRenderDrawColor(rend, 16, 16, 16, 0);
	SDL_RenderClear(rend);

	SDL_Rect rect = { .x = 10,.y = 10,.w = width / 2 - 15,.h = height - 20 };
	SDL_SetRenderDrawColor(rend, 32, 32, 32, 0);
	SDL_RenderFillRect(rend, &rect);
	SDL_Rect rect2 = { .x = width / 2 + 5,.y = 10,.w = width / 2 - 15,.h = height / 2 - 15 };
	SDL_RenderFillRect(rend, &rect2);
	SDL_Rect rect3 = { .x = width / 2 + 5,.y = height / 2 + 5,.w = width / 2 - 15,.h = height / 2 - 15 };
	SDL_RenderFillRect(rend, &rect3);
}

int main()
{
	// retutns zero on success else non-zero 
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("error initializing SDL: %s\n", SDL_GetError());
	}
	SDL_Window* win = SDL_CreateWindow("NES", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);

	// creates a renderer to render our images 
	SDL_Renderer* rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

	DrawScreen(rend);
	

	// Swap framebuffers
	SDL_RenderPresent(rend);

	SDL_Event event;
	while (event.type != SDL_QUIT)
	{
		SDL_Delay(10);
		SDL_PollEvent(&event);
	}

	// Cleanup
	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}
