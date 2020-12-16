#include "Gamepad.h"
#include "Frontend/RuntimeSettings.h"

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
	
	RuntimeSettings* rts = GetRuntimeSettings();

	pad->current_input.keys.A = state[rts->key_A];
	pad->current_input.keys.B = state[rts->key_B];
	pad->current_input.keys.Start = state[rts->key_start]; // Enter key
	pad->current_input.keys.Select = state[rts->key_select];
	pad->current_input.keys.Up = state[rts->key_up];
	pad->current_input.keys.Down = state[rts->key_down];
	pad->current_input.keys.Left = state[rts->key_left];
	pad->current_input.keys.Right = state[rts->key_right];
}
