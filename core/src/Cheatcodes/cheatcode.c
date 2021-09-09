#include "cheatcode.h"
#include <string.h>

uint8_t cheat_code_lut(char ch)
{
	switch (ch)
	{
	case 'A':
		return 0x0;
	case 'P':
		return 0x1;
	case 'Z':
		return 0x2;
	case 'L':
		return 0x3;
	case 'G':
		return 0x4;
	case 'I':
		return 0x5;
	case 'T':
		return 0x6;
	case 'Y':
		return 0x7;
	case 'E':
		return 0x8;
	case 'O':
		return 0x9;
	case 'X':
		return 0xA;
	case 'U':
		return 0xB;
	case 'K':
		return 0xC;
	case 'S':
		return 0xD;
	case 'V':
		return 0xE;
	case 'N':
		return 0xF;
	default:
		return 0x10;
	}
}

int initialize_cheat_code(CheatCode* cheat, char* code)
{
	int len = strlen(code);

	if (len != 6 && len != 8)
	{
		return 1;
	}

	cheat->use_compare = (len == 8);

	memset(cheat->code, 0, sizeof(cheat->code));

	int n[8];
	for (int i = 0; i < len; i++)
	{
		cheat->code[i] = code[i];
		n[i] = cheat_code_lut(code[i]);
		if (n[i] == 0x10)
			return 1; // failed
	}

	cheat->addr = 0x8000 | ((n[3] & 7) << 12) | ((n[5] & 7) << 8) | ((n[4] & 8) << 8) | ((n[2] & 7) << 4) | ((n[1] & 8) << 4) | (n[4] & 7) | (n[3] & 8);

	if (len == 6)
	{
		cheat->data = ((n[1] & 7) << 4) | ((n[0] & 8) << 4) | (n[0] & 7) | (n[5] & 8);
	}
	else
	{
		cheat->data = ((n[1] & 7) << 4) | ((n[0] & 8) << 4) | (n[0] & 7) | (n[7] & 8);
		cheat->compare = ((n[7] & 7) << 4) | ((n[6] & 8) << 4) | (n[6] & 7) | (n[5] & 8);
	}

	return 0;
}

uint8_t cheat_code_read(CheatCode* cheat, uint16_t addr, uint8_t compare, bool* read)
{
	*read = (cheat->addr == addr) && (!cheat->use_compare || cheat->compare == compare);
	
	return cheat->data;
}

void push_cheat_code(CheatCodeSystem* sys, char* code)
{
	if (sys->push_index <= 16)
	{
		if (initialize_cheat_code(&sys->codes[sys->push_index], code) == 0)
		{
			sys->push_index++;
		}
	}
}

void remove_cheat_code(CheatCodeSystem* sys, int index)
{
	if (index >= 0 && index < sys->push_index)
	{
		sys->push_index--;
		for (int i = index; i < sys->push_index; i++)
		{
			memcpy(&sys->codes[i], &sys->codes[i + 1], sizeof(CheatCode));
		}
	}
}

uint8_t cheat_code_read_system(CheatCodeSystem* sys, uint16_t addr, uint8_t compare, bool* read)
{
	*read = false;
	for (int i = 0; i < sys->push_index; i++)
	{
		int data = cheat_code_read(&sys->codes[i], addr, compare, read);
		if (*read)
			return data;
	}
	return 0;
}
