#ifndef GUI_H
#define GUI_H
// Quick and Dirty GUI system
#include <SDL.h>
#include <stdbool.h>

void GuiInit(SDL_Renderer* rend);
void GuiShutdown();

void GuiDispatchEvent(SDL_Event* e);
void GuiEndFrame(); // Call once per frame at the end of adding gui elements

bool GuiAddButton(const char* label, SDL_Rect* span);
bool GuiAddCheckbox(const char* label, int xoff, int yoff, bool* v);

#endif // !GUI_H
