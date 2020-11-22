#ifndef MAPPER_003_H
#define MAPPER_003_H
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "../Cartridge.h"
#include "MapperUtils.h"

typedef struct
{
	// Mapped from $8000-$FFFF
	uint8_t* PRG_ROM;

	uint8_t* CHR;

	MirrorMode mirrorMode;
	uint8_t CHR_bank_select;
	uint16_t PRG_ROM_banks;
	uint16_t CHR_banks;
	bool chr_is_ram;
} Mapper003;

void m003Free(Mapper003* mapper);
void m003LoadFromFile(Header* header, Cartridge* cart, FILE* file);

#endif // !MAPPER_003_H
