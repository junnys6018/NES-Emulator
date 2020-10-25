#include "Mapper_003.h"
#include "Frontend/Renderer.h" // To set pattern table
#include <string.h>

uint8_t m003CPUReadCartridge(void* mapper, uint16_t addr, bool* read)
{
	*read = (addr >= 0x4020 && addr <= 0xFFFF);

	Mapper003* map003 = (Mapper003*)mapper;
	if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		addr &= (map003->PRG_ROM_banks == 1 ? 0x3FFF : 0x7FFF);
		return map003->PRG_ROM[addr];
	}
}

uint8_t m003PPUReadCartridge(void* mapper, uint16_t addr)
{
	Mapper003* map003 = (Mapper003*)mapper;

	uint32_t index = (uint32_t)(map003->CHR_bank_select << 13) | addr;
	return map003->CHR[index];
}

void m003CPUWriteCartridge(void* mapper, uint16_t addr, uint8_t data, bool* wrote)
{
	*wrote = (addr >= 0x4020 && addr <= 0xFFFF);

	Mapper003* map003 = (Mapper003*)mapper;
	if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		map003->CHR_bank_select = data;
		uint32_t index = (uint32_t)(map003->CHR_bank_select << 13);
		RendererSetPatternTable(map003->CHR + index, 0);
		RendererSetPatternTable(map003->CHR + index + 0x1000, 1);
		assert(map003->CHR_bank_select < map003->CHR_banks);
	}
}

void m003PPUWriteCartridge(void* mapper, uint16_t addr, uint8_t data)
{
	Mapper003* map003 = (Mapper003*)mapper;

	uint32_t index = (uint32_t)(map003->CHR_bank_select << 13) | addr;
	map003->CHR[index] = data;
}

NametableIndex m003PPUMirrorNametable(void* mapper, uint16_t addr)
{
	Mapper003* map003 = (Mapper003*)mapper;
	switch (map003->mirrorMode)
	{
	case HORIZONTAL:
		return MirrorHorizontal(addr);
	case VERTICAL:
		return MirrorVertical(addr);
	}
}

void m003Free(Mapper003* mapper)
{
	free(mapper->PRG_ROM);
	free(mapper->CHR);
	free(mapper);
}

void m003LoadFromFile(Header* header, Cartridge* cart, FILE* file)
{
	cart->CPUReadCartridge = m003CPUReadCartridge;
	cart->PPUReadCartridge = m003PPUReadCartridge;

	cart->CPUWriteCartridge = m003CPUWriteCartridge;
	cart->PPUWriteCartridge = m003PPUWriteCartridge;

	cart->PPUMirrorNametable = m003PPUMirrorNametable;

	Mapper003* map = malloc(sizeof(Mapper003));
	assert(map);
	memset(map, 0, sizeof(Mapper003));
	cart->mapper = map;

	// Skip trainer if present
	if (header->Trainer)
	{
		fseek(file, 512, SEEK_CUR);
	}

	map->PRG_ROM_banks = ((uint16_t)header->PRGROM_MSB << 8) | (uint16_t)header->PRGROM_LSB;
	assert(map->PRG_ROM_banks == 1 || map->PRG_ROM_banks == 2);

	map->CHR_banks = ((uint16_t)header->CHRROM_MSB << 8) | (uint16_t)header->CHRROM_LSB;
	if (map->CHR_banks == 0)
	{
		map->CHR_banks = 1; // CHR is a RAM
	}

	map->PRG_ROM = malloc(((int)map->PRG_ROM_banks) * 16 * 1024);
	map->CHR = malloc(((int)map->CHR_banks) * 8 * 1024);

	fread(map->PRG_ROM, ((int)map->PRG_ROM_banks) * 16 * 1024, 1, file);
	fread(map->CHR, ((int)map->CHR_banks) * 8 * 1024, 1, file);

	// Set the mirror mode
	map->mirrorMode = header->MirrorType == 1 ? VERTICAL : HORIZONTAL;
}
