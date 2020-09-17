#ifndef CARTRIDGE_H
#define CARTRIDGE_H
#include <stdint.h>

typedef uint8_t(*READ_BYTE)(void* mapper, uint16_t addr);
typedef void(*WRITE_BYTE)(void* mapper, uint16_t addr, uint8_t data);

typedef uint16_t(*NAMETABLE_MIRROR)(void* mapper, uint16_t addr);

typedef struct
{
	uint16_t mapperID;
	READ_BYTE CPUReadCartridge;
	READ_BYTE PPUReadCartridge;

	WRITE_BYTE CPUWriteCartridge;
	WRITE_BYTE PPUWriteCartridge;

	NAMETABLE_MIRROR PPUMirrorNametable;

	void* mapper;
} Cartridge;

void load_cartridge_from_file(Cartridge* cart, const char* filepath);
void free_cartridge(Cartridge* cart);

#endif // !CARTRIDGE_H
