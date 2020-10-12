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
} Mapper003;

uint8_t m003CPUReadCartridge(void* mapper, uint16_t addr, bool* read);
uint8_t m003PPUReadCartridge(void* mapper, uint16_t addr);

void m003CPUWriteCartridge(void* mapper, uint16_t addr, uint8_t data, bool* wrote);
void m003PPUWriteCartridge(void* mapper, uint16_t addr, uint8_t data);

NametableIndex m003PPUMirrorNametable(void* mapper, uint16_t addr);

void m003Free(Mapper003* mapper);
void m003LoadFromFile(Header* header, Cartridge* cart, FILE* file);


#endif // !MAPPER_003_H
