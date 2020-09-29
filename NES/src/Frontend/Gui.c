#include "Gui.h"
#include "Renderer.h"

#include <stdbool.h>
#include <stdlib.h>

#include <stb_ds.h>

static SDL_Renderer* rend;
static UIElement* v_elements = NULL;
static unsigned int next_id = 1;

SDL_Color button_high = { 66, 150,250 };
SDL_Color button_low = { 35,69,109 };
SDL_Color checkbox_high = { 35,69,109 };
SDL_Color checkbox_low = { 29,47,73 };
SDL_Color checkbox_active = { 66,150,250 };

// UI components

// Button
void ButtonDraw(bool is_focus, SDL_Rect* span, void* data)
{
	ButtonElement* button = data;
	SDL_Color c = is_focus ? button_high : button_low;

	SDL_SetRenderDrawColor(rend, c.r, c.g, c.b, 255);
	SDL_RenderFillRect(rend, span);

	int xoff = span->x + (span->w - TextLen(button->label)) / 2;
	int yoff = span->y + (span->h - 15) / 2; // Font height is fixed at 15, this might be refactored to have a variable font size at some point TODO

	RenderText(button->label, white, xoff, yoff);
}

// Checkbox
void OnCheckboxClick(UIElement* elem, SDL_Event* e)
{
	CheckboxElement* checkbox = elem->data;
	checkbox->active = !checkbox->active;

	if (checkbox->on_click)
		checkbox->on_click(elem, e);
}

void CheckboxDraw(bool is_focus, SDL_Rect* span, void* data)
{
	CheckboxElement* checkbox = data;
	SDL_Color c = is_focus ? checkbox_high : checkbox_low;

	SDL_SetRenderDrawColor(rend, c.r, c.g, c.b, 255);
	SDL_RenderFillRect(rend, span);

	if (checkbox->active)
	{
		SDL_Rect r = { span->x + 5, span->y + 5, 10,10 };
		SDL_SetRenderDrawColor(rend, checkbox_active.r, checkbox_active.g, checkbox_active.b, 255);
		SDL_RenderFillRect(rend, &r);
	}
}

// Integer Input
typedef struct
{
	int* p;
	char buf[32];
} IntInputElement;

void OnIntInputClick(UIElement* elem, SDL_Event* e)
{

}

void IntInputDraw(bool is_focus, SDL_Rect* span, void* data)
{

}


// Public API implementation

void GuiInit(SDL_Renderer* r)
{
	rend = r;
}

void GuiShutdown()
{
	UIElement elem;
	for (int i = 0; i < arrlen(v_elements); i++)
	{
		elem = v_elements[i];
		free(elem.data);
	}
	arrfree(v_elements);
}

void GuiDraw()
{
	UIElement elem;
	for (int i = 0; i < arrlen(v_elements); i++)
	{
		elem = v_elements[i];
		elem.draw(elem.is_focus, &elem.span, elem.data);
	}
}

void GuiDispatchEvent(SDL_Event* e)
{
	switch (e->type)
	{
	case SDL_MOUSEBUTTONUP:
	{
		Sint32 x = e->button.x;
		Sint32 y = e->button.y;
		for (int i = 0; i < arrlen(v_elements); i++)
		{
			SDL_Rect r = v_elements[i].span;
			if (x >= r.x && y >= r.y && x <= r.x + r.w && y <= r.y + r.h)
			{
				if (v_elements[i].on_click)
				{
					v_elements[i].on_click(&v_elements[i], e);
				}
			}
		}
	}
	case SDL_MOUSEMOTION:
	{
		Sint32 x = e->motion.x;
		Sint32 y = e->motion.y;
		for (int i = 0; i < arrlen(v_elements); i++)
		{
			SDL_Rect r = v_elements[i].span;
			v_elements[i].is_focus = (x >= r.x && y >= r.y && x <= r.x + r.w && y <= r.y + r.h);
		}
	}
	}
}

unsigned int GuiAddButton(const char* label, SDL_Rect* span, ON_CLICK_EVENT callback)
{
	UIElement elem;
	elem.type = UI_BUTTON;
	elem.id = next_id++;
	elem.is_focus = false;
	elem.on_click = callback;
	elem.span = *span;
	elem.data = malloc(sizeof(ButtonElement));
	elem.draw = ButtonDraw;

	ButtonElement* data = elem.data;
	data->label = label;

	arrput(v_elements, elem);

	return elem.id;
}

unsigned int GuiAddCheckbox(int xoff, int yoff, ON_CLICK_EVENT callback)
{
	UIElement elem;
	elem.type = UI_CHECKBOX;
	elem.id = next_id++;
	elem.is_focus = false;
	elem.on_click = OnCheckboxClick;
	elem.span.x = xoff; elem.span.y = yoff; elem.span.w = 20; elem.span.h = 20;
	elem.data = malloc(sizeof(CheckboxElement));
	elem.draw = CheckboxDraw;

	CheckboxElement* data = elem.data;
	data->on_click = callback;

	arrput(v_elements, elem);

	return elem.id;
}

unsigned int GuiAddIntInput(int xoff, int yoff, int min, int max, int* p)
{
	return 0;
}

void GuiRemoveUIElement(unsigned int id)
{
	// Linear search through vector of UI-elements
	UIElement elem;
	for (int i = 0; i < arrlen(v_elements); i++)
	{
		elem = v_elements[i];
		if (elem.id == id)
		{
			free(elem.data);
			arrdel(v_elements, i);
			break;
		}
	}
}
