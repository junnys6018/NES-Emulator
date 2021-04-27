#ifndef STARTUP_OPTIONS_H
#define STARTUP_OPTIONS_H
#include <stdbool.h>
#include <SDL.h>

typedef struct
{
	SDL_Scancode key_A;
	SDL_Scancode key_B;
	SDL_Scancode key_start;
	SDL_Scancode key_select;
	SDL_Scancode key_up;
	SDL_Scancode key_down;
	SDL_Scancode key_left;
	SDL_Scancode key_right;

	bool fullscreen_on_startup;

	int startup_width, startup_height;

	int font_size;
	char font_style[256];
} StartupOptions;

StartupOptions* GetStartupOptions();
void LoadStartupOptions();

#endif