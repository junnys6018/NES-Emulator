#ifndef SETTINGS_MODEL_H
#define SETTINGS_MODEL_H
#include <stdbool.h>

typedef enum
{
	MODE_PLAY = 0,
	MODE_STEP_THROUGH = 1,
	MODE_NOT_RUNNING
} EmulationMode;

typedef struct
{
	float ms_per_frame;
	EmulationMode mode;
	bool fullscreen;
	bool draw_grid;
	bool scanline;
} SettingsModel;

#endif
