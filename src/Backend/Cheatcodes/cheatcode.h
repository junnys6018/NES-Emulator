#ifndef CHEAT_CODE_H
#define CHEAT_CODE_H
#include <stdint.h>
#include <stdbool.h>

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

void PushCheatCode(CheatCodeSystem* sys, char* code);
void RemoveCheatCode(CheatCodeSystem* sys, int index);
uint8_t CheatCodeReadSystem(CheatCodeSystem* sys, uint16_t addr, uint8_t compare, bool* read);

#endif // !CHEAT_CODE_H
