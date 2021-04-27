#include "AboutView.h"
#include "Common.h"
#include <stdio.h>

void DrawAbout()
{
	WindowMetrics* wm = GetWindowMetrics();
	int xoff = wm->db_x;
	int yoff = wm->db_y + wm->menu_button_h;

	int padding = wm->padding;
	SetTextOrigin(xoff + padding, yoff + padding);
	RenderText("NES Emulator by Jun Lim", cyan);
	RenderText("Controls", white);
	RenderText("In Step Through Mode:", cyan);
	RenderText("Space - Emulate one CPU instruction", white);
	RenderText("f     - Emulate one frame", white);
	RenderText("p     - Emulate one master clock cycle", white);
	RenderText("While Running Emulation", cyan);

	char buf[128];
	StartupOptions* opt = GetStartupOptions();
	const int pad = -10;

	sprintf(buf, "%*s - A button", pad, SDL_GetScancodeName(opt->key_A));
	RenderText(buf, white);

	sprintf(buf, "%*s - B button", pad, SDL_GetScancodeName(opt->key_B));
	RenderText(buf, white);

	sprintf(buf, "%*s - Start", pad, SDL_GetScancodeName(opt->key_start));
	RenderText(buf, white);

	sprintf(buf, "%*s - Select", pad, SDL_GetScancodeName(opt->key_select));
	RenderText(buf, white);

	sprintf(buf, "%*s - Up", pad, SDL_GetScancodeName(opt->key_up));
	RenderText(buf, white);

	sprintf(buf, "%*s - Down", pad, SDL_GetScancodeName(opt->key_down));
	RenderText(buf, white);

	sprintf(buf, "%*s - Left", pad, SDL_GetScancodeName(opt->key_left));
	RenderText(buf, white);

	sprintf(buf, "%*s - Right", pad, SDL_GetScancodeName(opt->key_right));
	RenderText(buf, white);
}
