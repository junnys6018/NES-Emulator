#ifndef MAPPER_002_H
#define MAPPER_002_H
#include "../Cartridge.h"
#include "MapperUtils.h"
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
	// Mapped from $8000-$FFFF
	uint8_t* PRG_ROM;

	// Fixed 8 KB CHR
	uint8_t CHR[8 * 1024];

	MirrorMode mirrorMode;
	uint8_t PRG_bank_select;
	uint8_t PRG_ROM_banks;
	bool chr_is_ram;
} Mapper002;

void m002Free(Mapper002* mapper);
void m002LoadFromFile(Header* header, Cartridge* cart, FILE* file);

#endif // !MAPPER_002_H
