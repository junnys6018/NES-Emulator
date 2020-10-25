#ifndef CONTROLLER_H
#define CONTROLLER_H

typedef enum
{
	MODE_PLAY = 0, MODE_STEP_THROUGH = 1, MODE_NOT_RUNNING
} EmulationMode;

// Struct used to pass data between the renderer and the main game loop
typedef struct
{
	float fps;
	float ms_per_frame;
	EmulationMode mode;
} Controller;
#endif // !CONTROLLER_H
