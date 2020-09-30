#include "Gui.h"
#include "Renderer.h"

#include <stdbool.h>
#include <stdlib.h>

#include <stb_ds.h>

typedef struct
{
	SDL_Renderer* rend;

	int mouse_x, mouse_y;
	bool mouse_pressed, mouse_released;
} GuiContext;

static GuiContext gc;

SDL_Color button_high = { 66, 150,250 };
SDL_Color button_low = { 35,69,109 };
SDL_Color checkbox_high = { 35,69,109 };
SDL_Color checkbox_low = { 29,47,73 };
SDL_Color checkbox_active = { 66,150,250 };

void GuiInit(SDL_Renderer* r)
{
	gc.rend = r;
}

void GuiShutdown()
{

}

void GuiDispatchEvent(SDL_Event* e)
{
	switch (e->type)
	{
	case SDL_MOUSEBUTTONUP:
		gc.mouse_released = true;
		break;
	case SDL_MOUSEBUTTONDOWN:
		gc.mouse_pressed = true;
		break;
	case SDL_MOUSEMOTION:
		gc.mouse_x = e->motion.x;
		gc.mouse_y = e->motion.y;
		break;
	}
}
void GuiEndFrame()
{
	gc.mouse_pressed = false;
	gc.mouse_released = false;
}

bool RectIntersectMouse(SDL_Rect* rect)
{
	return gc.mouse_x >= rect->x && gc.mouse_x <= rect->x + rect->w && gc.mouse_y >= rect->y && gc.mouse_y <= rect->y + rect->h;
}

bool GuiAddButton(const char* label, SDL_Rect* span)
{
	bool hover = RectIntersectMouse(span);
	SDL_Color c = hover ? button_high : button_low;

	SDL_SetRenderDrawColor(gc.rend, c.r, c.g, c.b, 255);
	SDL_RenderFillRect(gc.rend, span);

	int len = TextLen(label);
	RenderText(label, white, span->x + (span->w - len) / 2, span->y + (span->h - 15 + 1) / 2);

	return hover && gc.mouse_released;
}

bool GuiAddCheckbox(const char* label, int xoff, int yoff, bool* v)
{
	SDL_Rect span = { xoff,yoff,20,20 };
	bool hover = RectIntersectMouse(&span);
	SDL_Color c = hover ? checkbox_high : checkbox_low;

	SDL_SetRenderDrawColor(gc.rend, c.r, c.g, c.b, 255);
	SDL_RenderFillRect(gc.rend, &span);

	bool pressed = hover && gc.mouse_released;
	if (pressed)
	{
		*v = !*v;
	}

	if (*v)
	{
		SDL_SetRenderDrawColor(gc.rend, checkbox_active.r, checkbox_active.g, checkbox_active.b, 255);
		span.x += 5; span.y += 5; span.h = 10; span.w = 10;
		SDL_RenderFillRect(gc.rend, &span);
	}

	if (label)
	{
		RenderText(label, white, xoff + 25, yoff + 2);
	}

	return pressed;
}
