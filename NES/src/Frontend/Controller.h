#ifndef CONTROLLER_H
#define CONTROLLER_H

typedef enum
{
	MODE_PLAY = 0, MODE_STEP_THROUGH = 1, MODE_NOT_RUNNING
} EmulationMode;

typedef struct
{
	EmulationMode mode;
} Controller;
#endif // !CONTROLLER_H