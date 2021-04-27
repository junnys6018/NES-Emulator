#ifndef MAPPER_JUN_H
#define MAPPER_JUN_H
#include <stdint.h>
#include <stdio.h>
#include "../Cartridge.h"
#include "MapperUtils.h"

typedef struct
{
	uint8_t PRG_RAM[64 * 1024];
	uint8_t CHR[8 * 1024];
} MapperJUN;

uint8_t mJUNCPUReadCartridge(void* mapper, uint16_t addr, bool* read);
void mJUNCPUWriteCartridge(void* mapper, uint16_t addr, uint8_t data, bool* wrote);

uint8_t mJUNPPUReadCartridge(void* mapper, uint16_t addr);
void mJUNPPUWriteCartridge(void* mapper, uint16_t addr, uint8_t data);

NametableIndex mJUNPPUMirrorNametable(void* mapper, uint16_t addr);

void mJUNLoadFromFile(Cartridge* cart, FILE* file);

#endif // !MAPPER_000_H