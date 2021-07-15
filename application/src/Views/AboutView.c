#include "AboutView.h"
#include "Common.h"
#include <stdio.h>

static const char* xbox_btns[] = {"A", "B", "Start", "Back", "D-pad Up", "D-pad Down", "D-pad Left", "D-pad Right"};
static const char* ps4_btns[] = {"X", "O", "Options", "Share", "D-pad Up", "D-pad Down", "D-pad Left", "D-pad Right"};

void DrawControls(const char** names)
{
	char buf[128];

	sprintf(buf, "A button - %s", names[0]);
	RenderText(buf, white);

	sprintf(buf, "B button - %s", names[1]);
	RenderText(buf, white);

	sprintf(buf, "Start    - %s", names[2]);
	RenderText(buf, white);

	sprintf(buf, "Select   - %s", names[3]);
	RenderText(buf, white);

	sprintf(buf, "Up       - %s", names[4]);
	RenderText(buf, white);

	sprintf(buf, "Down     - %s", names[5]);
	RenderText(buf, white);

	sprintf(buf, "Left     - %s", names[6]);
	RenderText(buf, white);

	sprintf(buf, "Right    - %s", names[7]);
	RenderText(buf, white);
}

void DrawAbout(SDL_GameControllerType type)
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

	StartupOptions* opt = GetStartupOptions();
	const char* names[8];
	names[0] = SDL_GetScancodeName(opt->key_A);
	names[1] = SDL_GetScancodeName(opt->key_B);
	names[2] = SDL_GetScancodeName(opt->key_start);
	names[3] = SDL_GetScancodeName(opt->key_select);
	names[4] = SDL_GetScancodeName(opt->key_up);
	names[5] = SDL_GetScancodeName(opt->key_down);
	names[6] = SDL_GetScancodeName(opt->key_left);
	names[7] = SDL_GetScancodeName(opt->key_right);

	RenderText("Keyboard", cyan);
	DrawControls(names);

	// Display different buttons based on the controller (xbox or ps4)
	const char** controller_names = xbox_btns; // Default to xbox
	if (type == SDL_CONTROLLER_TYPE_PS3 || type == SDL_CONTROLLER_TYPE_PS4)
	{
		controller_names = ps4_btns;
	}
	RenderText("Controller", cyan);
	DrawControls(controller_names);
}
