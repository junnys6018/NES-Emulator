#ifndef MAPPER_000_H
#define MAPPER_000_H
#include <stdint.h>
#include "../Cartridge.h"

typedef enum 
{
	HORIZONTAL, VERTICAL
} MirrorMode;

typedef enum
{
	_16K, _32K
} RomCapacity;


typedef struct
{
	uint8_t* PRG_ROM;
	uint8_t PRG_RAM[8 * 1024];
	uint8_t CHR[8 * 1024];

	MirrorMode mirrorMode;
	RomCapacity romCap;
} Mapper000;

uint8_t m000CPUReadCartridge(void* mapper, uint16_t addr, bool* read);
uint8_t m000PPUReadCartridge(void* mapper, uint16_t addr);

void m000CPUWriteCartridge(void* mapper, uint16_t addr, uint8_t data, bool* wrote);
void m000PPUWriteCartridge(void* mapper, uint16_t addr, uint8_t data);

NametableIndex m000PPUMirrorNametable(void* mapper, uint16_t addr);

void m000Free(Mapper000* mapper);


#endif // !MAPPER_000_H
