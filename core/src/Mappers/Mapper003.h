#ifndef MAPPER_003_H
#define MAPPER_003_H
#include "Cartridge.h"
#include "MapperUtils.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
	// Mapped from $8000-$FFFF
	uint8_t* PRG_ROM;

	uint8_t* CHR;

	MirrorMode mirror_mode;
	uint8_t CHR_bank_select;
	uint16_t PRG_ROM_banks;
	uint16_t CHR_banks;
	bool chr_is_ram;
} Mapper003;

void m003_free(Mapper003* mapper);
void m003_load_from_file(Header* header, Cartridge* cart, FILE* file);

#endif
