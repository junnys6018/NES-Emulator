#ifndef CHEAT_CODE_H
#define CHEAT_CODE_H
#include <stdbool.h>
#include <stdint.h>

// See http://tuxnes.sourceforge.net/gamegenie.html
typedef struct
{
	uint16_t addr;
	uint8_t data;
	uint8_t compare;
	bool use_compare;

	char code[9]; // null terminated string
} CheatCode;

typedef struct
{
	CheatCode codes[16];
	int push_index;
} CheatCodeSystem;

void push_cheat_code(CheatCodeSystem* sys, char* code);
void remove_cheat_code(CheatCodeSystem* sys, int index);
uint8_t cheat_code_read_system(CheatCodeSystem* sys, uint16_t addr, uint8_t compare, bool* read);

#endif
