#include "Mapper_JUN.h"
#include <stdlib.h>
#include <assert.h>

uint8_t mJUNCPUReadCartridge(Cartridge* cart, uint16_t addr, bool* read)
{
	*read = true;
	MapperJUN* mapJUN = (MapperJUN*)cart->mapper;

	return mapJUN->PRG_RAM[addr];
}

void mJUNCPUWriteCartridge(Cartridge* cart, uint16_t addr, uint8_t data, bool* wrote)
{
	*wrote = true;
	MapperJUN* mapJUN = (MapperJUN*)cart->mapper;
	
	mapJUN->PRG_RAM[addr] = data;
}

uint8_t mJUNPPUReadCartridge(Cartridge* cart, uint16_t addr)
{
	MapperJUN* mapJUN = (MapperJUN*)cart->mapper;
	return mapJUN->CHR[addr];
}

void mJUNPPUWriteCartridge(Cartridge* cart, uint16_t addr, uint8_t data)
{
	MapperJUN* mapJUN = (MapperJUN*)cart->mapper;
	mapJUN->CHR[addr] = data;
}

// Fixed horizontal mirroing
NametableIndex mJUNPPUMirrorNametable(void* mapper, uint16_t addr)
{
	return MirrorHorizontal(addr);
}

void mJUNLoadFromFile(Cartridge* cart, FILE* file)
{
	cart->mapperID = 767; // Assign mapperID 767 to my format

	cart->CPUReadCartridge = mJUNCPUReadCartridge;
	cart->CPUWriteCartridge = mJUNCPUWriteCartridge;

	cart->PPUReadCartridge = mJUNPPUReadCartridge;
	cart->PPUPeakCartridge = mJUNPPUReadCartridge;
	cart->PPUWriteCartridge = mJUNPPUWriteCartridge;

	cart->PPUMirrorNametable = mJUNPPUMirrorNametable;

	MapperJUN* map = malloc(sizeof(MapperJUN));
	assert(map);
	cart->mapper = map;

	fread(map->PRG_RAM, 64 * 1024, 1, file);
}
