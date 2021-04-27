#include "Gamepad.h"

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

void poll_keys(Gamepad* pad, Keys keys)
{
	pad->current_input.reg = keys.reg;
}
