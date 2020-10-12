#ifndef GAMEPAD_H
#define GAMEPAD_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
	bool strobe;
	uint8_t shift_reg_out;
	union
	{
		struct
		{
			uint8_t A : 1;
			uint8_t B : 1;
			uint8_t Select : 1;
			uint8_t Start : 1;
			uint8_t Up : 1;
			uint8_t Down : 1;
			uint8_t Left : 1;
			uint8_t Right : 1;
		} keys;
		uint8_t reg;
	}current_input;
} Gamepad;

uint8_t gamepad_read(Gamepad* pad);
void gamepad_write(Gamepad* pad, uint8_t data);
void poll_keys(Gamepad* pad);

#endif // !GAMEPAD_H
