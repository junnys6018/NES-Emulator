#include "Gamepad.h"
#include <SDL.h>

// TODO: open bus behaviour

uint8_t gamepad_read(Gamepad* pad)
{
	if (pad->strobe)
	{
		pad->shift_reg_out = pad->current_input.reg;
	}
	uint8_t ret = pad->shift_reg_out & 0x01;
	pad->shift_reg_out = (pad->shift_reg_out >> 1) | 0x80; // 1 is fed into leftmost bit
	return ret;
}

void gamepad_write(Gamepad* pad, uint8_t data)
{
	pad->strobe = data & 0x01;
	if (pad->strobe)
	{
		pad->shift_reg_out = pad->current_input.reg;
	}
}

void poll_keys(Gamepad* pad)
{
	static const int8_t* state = NULL;
	if (!state)
	{
		state = SDL_GetKeyboardState(NULL);
	}
	
	pad->current_input.keys.A = state[SDL_SCANCODE_X];
	pad->current_input.keys.B = state[SDL_SCANCODE_Z];
	pad->current_input.keys.Start = state[SDL_SCANCODE_RETURN]; // Enter key
	pad->current_input.keys.Select = state[SDL_SCANCODE_TAB];
	pad->current_input.keys.Up = state[SDL_SCANCODE_UP];
	pad->current_input.keys.Down = state[SDL_SCANCODE_DOWN];
	pad->current_input.keys.Left = state[SDL_SCANCODE_LEFT];
	pad->current_input.keys.Right = state[SDL_SCANCODE_RIGHT];
}
