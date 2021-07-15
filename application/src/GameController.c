#include "GameController.h"
#include <stdio.h>

void TryOpenGameController(GameController* game_controller)
{
	game_controller->sdl_game_controller = NULL;
	for (int i = 0; i < SDL_NumJoysticks(); i++)
	{
		if (SDL_IsGameController(i))
		{
			game_controller->sdl_game_controller = SDL_GameControllerOpen(i);
			if (game_controller->sdl_game_controller)
				break;
			else
				printf("Could not open gamecontroller %i: %s\n", i, SDL_GetError());
		}
	}
}

void CloseGameController(GameController* game_controller)
{
	if (IsGameControllerOpen(game_controller))
		SDL_GameControllerClose(game_controller->sdl_game_controller);
}

Keys PollGameController(GameController* game_controller)
{
	Keys ret;
	ret.reg = 0;
	if (!IsGameControllerOpen(game_controller))
	{
		return ret;
	}

	SDL_GameController* gc = game_controller->sdl_game_controller;
	ret.keys.A = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_A);
	ret.keys.B = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_B);
	ret.keys.Start = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_START);
	ret.keys.Select = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_BACK);
	ret.keys.Up = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_DPAD_UP);
	ret.keys.Down = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
	ret.keys.Left = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
	ret.keys.Right = SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);

	return ret;
}
