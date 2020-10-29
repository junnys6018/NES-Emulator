#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <SDL.h>

typedef enum
{
	MODE_PLAY = 0, MODE_STEP_THROUGH = 1, MODE_NOT_RUNNING
} EmulationMode;

// Struct used to pass data between the renderer and the main game loop
typedef struct
{
	EmulationMode mode;
	SDL_AudioDeviceID audio_id;
} Controller;
#endif // !CONTROLLER_H
