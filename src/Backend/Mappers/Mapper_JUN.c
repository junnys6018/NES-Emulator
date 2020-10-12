#include "Mapper_JUN.h"
#include <stdlib.h>
#include <assert.h>

uint8_t mJUNCPUReadCartridge(void* mapper, uint16_t addr, bool* read)
{
	*read = true;
	MapperJUN* mapJUN = (MapperJUN*)mapper;

	return mapJUN->PRG_RAM[addr];
}

uint8_t mJUNPPUReadCartridge(void* mapper, uint16_t addr)
{
	MapperJUN* mapJUN = (MapperJUN*)mapper;
	return mapJUN->CHR[addr];
}

void mJUNCPUWriteCartridge(void* mapper, uint16_t addr, uint8_t data, bool* wrote)
{
	*wrote = true;
	MapperJUN* mapJUN = (MapperJUN*)mapper;
	
	mapJUN->PRG_RAM[addr] = data;
}

void mJUNPPUWriteCartridge(void* mapper, uint16_t addr, uint8_t data)
{
	MapperJUN* mapJUN = (MapperJUN*)mapper;
	mapJUN->CHR[addr] = data;
}

// Fixed horizontal mirroing
NametableIndex mJUNPPUMirrorNametable(void* mapper, uint16_t addr)
{
	NametableIndex ret;
	ret.addr = addr & 0x3FF;

	if (addr >= 0x2000 && addr < 0x2800)
	{
		// Map into first table
		ret.index = 0;
	}
	else // (addr >= 0x2800 && addr < 0x3000)
	{
		// Map into second table
		ret.index = 1;
	}

	return ret;
}

void mJUNLoadFromFile(Cartridge* cart, FILE* file)
{
	cart->mapperID = 767; // Assigm mapperID 767 to my format

	cart->CPUReadCartridge = mJUNCPUReadCartridge;
	cart->PPUReadCartridge = mJUNPPUReadCartridge;

	cart->CPUWriteCartridge = mJUNCPUWriteCartridge;
	cart->PPUWriteCartridge = mJUNPPUWriteCartridge;

	cart->PPUMirrorNametable = mJUNPPUMirrorNametable;

	MapperJUN* map = malloc(sizeof(MapperJUN));
	assert(map);
	cart->mapper = map;

	fread(map->PRG_RAM, 64 * 1024, 1, file);
}
