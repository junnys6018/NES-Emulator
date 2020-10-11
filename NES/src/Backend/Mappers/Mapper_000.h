#ifndef MAPPER_000_H
#define MAPPER_000_H
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "../Cartridge.h"
#include "MapperUtils.h"

typedef struct
{
	// Mapped from $6000-$7FFF
	uint8_t* PRG_ROM;

	// Mapped from $8000-$FFFF
	uint8_t PRG_RAM[8 * 1024];


	uint8_t CHR[8 * 1024];

	MirrorMode mirrorMode;
	uint16_t PRG_ROM_Banks;
} Mapper000;

uint8_t m000CPUReadCartridge(void* mapper, uint16_t addr, bool* read);
uint8_t m000PPUReadCartridge(void* mapper, uint16_t addr);

void m000CPUWriteCartridge(void* mapper, uint16_t addr, uint8_t data, bool* wrote);
void m000PPUWriteCartridge(void* mapper, uint16_t addr, uint8_t data);

NametableIndex m000PPUMirrorNametable(void* mapper, uint16_t addr);

void m000Free(Mapper000* mapper);
void m000LoadFromFile(Header* header, Cartridge* cart, FILE* file);


#endif // !MAPPER_000_H
