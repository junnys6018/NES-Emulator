#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H
#include <SDL_gamecontroller.h>
#include <stdbool.h>
#include <Gamepad.h>

typedef struct
{
	SDL_GameController* sdl_game_controller;
} GameController;

void TryOpenGameController(GameController* game_controller);
void CloseGameController(GameController* game_controller);
inline bool IsGameControllerOpen(GameController* game_controller)
{
	return game_controller->sdl_game_controller != NULL;
}

Keys PollGameController(GameController* game_controller);

#endif