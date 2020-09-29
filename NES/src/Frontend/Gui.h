#ifndef GUI_H
#define GUI_H
// Quick and Dirty GUI library
// Implmentation is slow and wont scale well, only intended to run on a few widgets
#include <SDL.h>
#include <stdbool.h>

void GuiInit(SDL_Renderer* rend);
void GuiShutdown();
void GuiDraw();

void GuiDispatchEvent(SDL_Event* e);

typedef enum
{
	UI_BUTTON, UI_CHECKBOX
} UIType;

struct UIElement; // Forward declaration

typedef void (*DRAW)(bool is_focus, SDL_Rect* span, void* data);
typedef void (*ON_CLICK_EVENT)(struct UIElement* elem, SDL_Event* e);

typedef struct
{
	UIType type;
	unsigned int id;
	bool is_focus;

	// Event callback handler
	ON_CLICK_EVENT on_click;

	SDL_Rect span;
	void* data;

	DRAW draw;
} UIElement;

typedef struct
{
	const char* label;
} ButtonElement;

typedef struct
{
	bool active;
	ON_CLICK_EVENT on_click;
} CheckboxElement;

unsigned int GuiAddButton(const char* label, SDL_Rect* span, ON_CLICK_EVENT callback);
unsigned int GuiAddCheckbox(int xoff, int yoff, ON_CLICK_EVENT callback);
unsigned int GuiAddIntInput(int xoff, int yoff, int min, int max, int* p);

void GuiRemoveUIElement(unsigned int id);


#endif // !GUI_H
