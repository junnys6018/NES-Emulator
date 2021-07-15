#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H
#include <SDL_gamecontroller.h>
#include <stdbool.h>
#include <Gamepad.h>

typedef struct
{
	SDL_GameController* sdl_game_controller;
	int device_index;
} GameController;

void TryOpenGameController(GameController* game_controller);
bool OpenGameController(GameController* game_controller, int device_index);
void CloseGameController(GameController* game_controller);
inline bool IsGameControllerOpen(GameController* game_controller)
{
	return game_controller->sdl_game_controller != NULL;
}

Keys PollGameController(GameController* game_controller);
SDL_GameControllerType GetControllerType(GameController* game_controller);

#endif