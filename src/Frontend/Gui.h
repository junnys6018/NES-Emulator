#ifndef GUI_H
#define GUI_H
// Quick and Dirty GUI system
#include <SDL.h>
#include <stdbool.h>

typedef struct
{
	int scroll_bar_width;
	int checkbox_size;
	int font_size;
	int padding;
} GuiMetrics;

void GuiInit(GuiMetrics* metrics);
void GuiShutdown();

void GuiDispatchEvent(SDL_Event* e);
void GuiEndFrame(); // Call once per frame at the end of adding gui elements
GuiMetrics* GetGuiMetrics();

bool GuiAddButton(const char* label, SDL_Rect* span);
bool GuiAddCheckbox(const char* label, int xoff, int yoff, bool* v);
bool GuiAddScrollBar(const char* label, SDL_Rect* span, int* v, int max, int scale);

#endif // !GUI_H
