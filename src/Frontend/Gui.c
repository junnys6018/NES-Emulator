#include "Gui.h"
#include "Renderer.h"

#include <stdbool.h>
#include <stdlib.h>

#include <stb_ds.h>

typedef struct
{
	SDL_Renderer* rend;

	GuiMetrics metrics;

	int mouse_x, mouse_y;
	bool mouse_pressed, mouse_released;
	int wheel;
} GuiContext;

static GuiContext gc;

SDL_Color button_high = { 66, 150,250 };
SDL_Color button_low = { 35,69,109 };
SDL_Color checkbox_high = { 35,69,109 };
SDL_Color checkbox_low = { 29,47,73 };
SDL_Color checkbox_active = { 66,150,250 };
SDL_Color scroll_bar = { 29,47,73 };
SDL_Color scroll_grab_low = { 66,150,250 };
SDL_Color scroll_grab_high = { 3,132,252 };

unsigned long hash(const unsigned char* str)
{
	unsigned long hash = 5381;
	int c;

	while (c = *str++)
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

void GuiInit(SDL_Renderer* r, GuiMetrics* metrics)
{
	gc.rend = r;
	memcpy(&gc.metrics, metrics, sizeof(GuiMetrics));
}

void GuiShutdown()
{

}

void GuiDispatchEvent(SDL_Event* e)
{
	switch (e->type)
	{
	case SDL_MOUSEWHEEL:
		gc.wheel -= e->wheel.y; // Invert y
		break;
	}
}

void GuiEndFrame()
{
	static bool last_state; // true: pressed; false: released
	Uint32 button_state = SDL_GetMouseState(&gc.mouse_x, &gc.mouse_y);
	bool current_state = button_state & SDL_BUTTON(SDL_BUTTON_LEFT) ? true : false;
	gc.mouse_pressed = current_state && !last_state;
	gc.mouse_released = !current_state && last_state;

	last_state = current_state;

	gc.wheel = 0;
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

	int len = TextBounds(label).w;
	SetTextOrigin(span->x + (span->w - len) / 2, span->y + (span->h - gc.metrics.font_size) / 2);
	RenderText(label, white);

	return hover && gc.mouse_released;
}

bool GuiAddCheckbox(const char* label, int xoff, int yoff, bool* v)
{
	SDL_Rect span = { xoff,yoff,gc.metrics.checkbox_size,gc.metrics.checkbox_size };
	bool hover = RectIntersectMouse(&span);
	SDL_Color c = hover ? checkbox_high : checkbox_low;

	// Render checkbox
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
		int offset = (int)roundf((float)gc.metrics.checkbox_size / 4);
		int width = gc.metrics.checkbox_size - 2 * offset;
		span.x += offset; span.y += offset; span.h = width; span.w = width;
		SDL_RenderFillRect(gc.rend, &span);
	}

	if (label)
	{
		SetTextOrigin(xoff + gc.metrics.checkbox_size + gc.metrics.padding, yoff + (gc.metrics.checkbox_size - gc.metrics.font_size) / 2);
		RenderText(label, white);
	}

	return pressed;
}

bool GuiAddScrollBar(const char* label, SDL_Rect* span, int* v, int max, int scale)
{
	// Is there an active grab?
	static bool is_active = false;
	// If so, whats the id of the active grab
	static unsigned long active_grab = 0;
	static int yoff = 0;

	bool activated = false;
	if (RectIntersectMouse(span) && !is_active)
	{
		int old_v = *v;
		*v += scale * gc.wheel;
		if (*v < 0)
		{
			* v = 0;
		}
		else if (*v > max)
		{
			*v = max;
		}
		activated = old_v != *v;
	}

	// Render bounding box
	SDL_SetRenderDrawColor(gc.rend, 32, 32, 32, 255);
	SDL_RenderFillRect(gc.rend, span);

	// Render scroll bar
	SDL_Rect bar = { span->x + span->w - gc.metrics.scroll_bar_width, span->y, gc.metrics.scroll_bar_width,span->h };
	SDL_SetRenderDrawColor(gc.rend, scroll_bar.r, scroll_bar.g, scroll_bar.b, 255);
	SDL_RenderFillRect(gc.rend, &bar);

	// Render the grab
	int height = 5 * span->h * scale / max;
	int x = span->x + span->w - gc.metrics.scroll_bar_width;
	if (is_active && hash(label) == active_grab)
	{
		int old_v = *v;
		*v = max * (gc.mouse_y + yoff - span->y) / (span->h - height);
		if (*v < 0)
		{
			*v = 0;
		}
		else if (*v > max)
		{
			*v = max;
		}
		activated = old_v != *v;
		int y = *v * (span->h - height) / max + span->y;

		SDL_Rect grab = { x,y,gc.metrics.scroll_bar_width,height };
		SDL_SetRenderDrawColor(gc.rend, scroll_grab_high.r, scroll_grab_high.g, scroll_grab_high.b, 255);
		SDL_RenderFillRect(gc.rend, &grab);
		if (gc.mouse_released)
		{
			is_active = false;
		}
	}
	else
	{
		int y = *v * (span->h - height) / max + span->y;
		SDL_Rect grab = { x,y,gc.metrics.scroll_bar_width,height };
		SDL_Color c = RectIntersectMouse(&grab) ? scroll_grab_high : scroll_grab_low;
		SDL_SetRenderDrawColor(gc.rend, c.r, c.g, c.b, 255);
		SDL_RenderFillRect(gc.rend, &grab);

		if (gc.mouse_pressed && RectIntersectMouse(&grab) && !is_active)
		{
			is_active = true;
			active_grab = hash(label);
			// Calculate offset to the top of the grab
			yoff = y - gc.mouse_y;
		}
	}

	return activated;
}