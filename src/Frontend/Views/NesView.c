#include "NesView.h"
#include "Common.h"

void DrawNes(NesScreenModel scr)
{
	WindowMetrics* wm = GetWindowMetrics();

	SDL_Rect r_NesView = {wm->nes_x, wm->nes_y, wm->nes_w, wm->nes_h};
	SubmitTexturedQuadf(&r_NesView, scr.scr.handle);
}