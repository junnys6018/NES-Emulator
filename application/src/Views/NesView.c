#include "NesView.h"
#include "Common.h"
#include "../OpenGL/Scanline.h"

void DrawNes(NesScreenModel scr, SettingsModel* set)
{
	WindowMetrics* wm = GetWindowMetrics();
	SDL_Rect r_NesView = {wm->nes_x, wm->nes_y, wm->nes_w, wm->nes_h};

	if (set->scanline)
	{
		float time = (float)SDL_GetTicks() / 1000.0f;
		GLuint out = ScanlineOnDraw(time, scr.scr.handle);

		SubmitTexturedQuadf(&r_NesView, out);
	}
	else
	{
		SubmitTexturedQuadf(&r_NesView, scr.scr.handle);
	}

	// Draw 8x8 grid over nes screen for debugging
	if (set->draw_grid)
	{
		int shade;
		for (int i = 0; i <= 32; i++)
		{
			if (i == 16)
				shade = 128;
			else if (i % 2 == 0)
				shade = 64;
			else
				shade = 32;

			int x = wm->nes_x + wm->nes_w * i / 32;
			SubmitLine(x, wm->nes_y, x, wm->nes_y + wm->nes_h, shade, shade, shade);
		}
		for (int i = 0; i <= 30; i++)
		{
			if (i == 15)
				shade = 128;
			else if (i % 2 == 0)
				shade = 64;
			else
				shade = 32;

			int y = wm->nes_y + wm->nes_w * i / 32;
			SubmitLine(wm->nes_x, y, wm->nes_x + wm->nes_w, y, shade, shade, shade);
		}
	}
}