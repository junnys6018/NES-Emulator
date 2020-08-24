#include <SDL.h> 
#include <SDL_ttf.h>
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

void TTF_Error(const char* message)
{
	printf("[TTF ERROR] %s: %s\n", message, TTF_GetError());
}

int main()
{
	// retutns zero on success else non-zero 
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) 
	{
		printf("[ERROR] Initializing SDL: %s\n", SDL_GetError());
	}

	if (TTF_Init() != 0)
	{
		TTF_Error("Initializing SDL_TTF");
	}

	TTF_Font* font = TTF_OpenFont("Consola.ttf", 14);
	if (!font)
	{
		TTF_Error("Cant Open Font");
	}

	SDL_Window* win = SDL_CreateWindow("NES", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);

	// creates a renderer to render our images 
	SDL_Renderer* rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

	DrawScreen(rend);

	SDL_Color c = { 255,255,255 };
	SDL_Surface* s_text = TTF_RenderUTF8_Blended(font, "Hello, World!", c);
	SDL_Texture* t_text = SDL_CreateTextureFromSurface(rend, s_text);
	SDL_Rect rect = { .x = 15,.y = 15,.w = s_text->w,.h = s_text->h };
	SDL_RenderCopy(rend, t_text, NULL, &rect);
	

	// Swap framebuffers
	SDL_RenderPresent(rend);

	SDL_Event event;
	while (event.type != SDL_QUIT)
	{
		SDL_Delay(10);
		SDL_PollEvent(&event);
	}

	// Cleanup
	TTF_CloseFont(font);
	TTF_Quit();

	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}
