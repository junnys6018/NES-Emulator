#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "nes.h"
#include "Models/SettingsModel.h"
#include <SDL.h>

void InitApplication(char* rom);
void ShutdownApplication();
void ApplicationGameLoop();

typedef struct
{
	int padding;

	int width, height; // width and height of window
	int db_x, db_y;	   // x and y offset of debug screen
	int db_w, db_h;	   // width and height of debug screen
	int nes_x, nes_y;  // x and y offset of nes screen
	int nes_w, nes_h;  // width and heigh of nes sceen

	// width and height of menu buttons
	float menu_button_w;
	int menu_button_h;

	int button_h; // height of normal buttons

	// width and height of pattern table visualisation
	int pattern_table_len;

	// length of each palette visualisation "box"
	int palette_visual_len;

	int apu_osc_height;
} WindowMetrics;

void GetWindowSize(int* w, int* h);
WindowMetrics* GetWindowMetrics();
Nes* GetApplicationNes();
void SetFullScreen(bool b);
void SetPatternTable(uint8_t* table_data, int side); // The emulator to call into
#endif