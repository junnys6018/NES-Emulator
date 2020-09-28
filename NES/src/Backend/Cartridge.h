#ifndef CARTRIDGE_H
#define CARTRIDGE_H
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t(*CPU_READ_BYTE)(void* mapper, uint16_t addr, bool* read);
typedef void(*CPU_WRITE_BYTE)(void* mapper, uint16_t addr, uint8_t data, bool* wrote);
typedef uint8_t(*PPU_READ_BYTE)(void* mapper, uint16_t addr);
typedef void(*PPU_WRITE_BYTE)(void* mapper, uint16_t addr, uint8_t data);

typedef struct
{
	uint16_t index : 1;
	uint16_t addr : 10;
} NametableIndex;
typedef NametableIndex(*NAMETABLE_MIRROR)(void* mapper, uint16_t addr);

typedef struct
{
	uint16_t mapperID;
	CPU_READ_BYTE CPUReadCartridge;
	PPU_READ_BYTE PPUReadCartridge;

	CPU_WRITE_BYTE CPUWriteCartridge;
	PPU_WRITE_BYTE PPUWriteCartridge;

	NAMETABLE_MIRROR PPUMirrorNametable;

	void* mapper;
} Cartridge;

void load_cartridge_from_file(Cartridge* cart, const char* filepath);
void free_cartridge(Cartridge* cart);

#endif // !CARTRIDGE_H
