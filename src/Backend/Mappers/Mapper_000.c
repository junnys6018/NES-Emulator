#include "Mapper_000.h"
#include "Frontend/Renderer.h"
#include <stdlib.h>
#include <string.h>

uint8_t m000CPUReadCartridge(void* mapper, uint16_t addr, bool* read)
{
	*read = (addr >= 0x4020 && addr <= 0xFFFF);

	Mapper000* map000 = (Mapper000*)mapper;
	// Read from PGR_RAM
	if (addr >= 0x6000 && addr < 0x8000)
	{
		addr &= 0x1FFF;
		return map000->PRG_RAM[addr];
	}
	// Read from PGR_ROM
	else if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		addr &= (map000->PRG_ROM_Banks == 1 ? 0x3FFF : 0x7FFF);
		return map000->PRG_ROM[addr];
	}
	return 0;
}

void m000CPUWriteCartridge(void* mapper, uint16_t addr, uint8_t data, bool* wrote)
{
	*wrote = (addr >= 0x4020 && addr <= 0xFFFF);

	Mapper000* map000 = (Mapper000*)mapper;
	// Write to PGR_RAM
	if (addr >= 0x6000 && addr < 0x8000)
	{
		addr &= 0x1FFF;
		map000->PRG_RAM[addr] = data;
	}
}

uint8_t m000PPUReadCartridge(void* mapper, uint16_t addr)
{
	Mapper000* map000 = (Mapper000*)mapper;
	return map000->CHR[addr];
}

void m000PPUWriteCartridge(void* mapper, uint16_t addr, uint8_t data)
{
	Mapper000* map000 = (Mapper000*)mapper;
	if (map000->chr_is_ram)
	{
		map000->CHR[addr] = data;
	}
}

NametableIndex m000PPUMirrorNametable(void* mapper, uint16_t addr)
{
	Mapper000* map000 = (Mapper000*)mapper;
	switch (map000->mirrorMode)
	{
	case HORIZONTAL:
		return MirrorHorizontal(addr);
	case VERTICAL:
		return MirrorVertical(addr);
	}
}

void m000Free(Mapper000* mapper)
{
	free(mapper->PRG_ROM);
	free(mapper);
}

void m000LoadFromFile(Header* header, Cartridge* cart, FILE* file)
{
	cart->CPUReadCartridge = m000CPUReadCartridge;
	cart->CPUWriteCartridge = m000CPUWriteCartridge;

	cart->PPUReadCartridge = m000PPUReadCartridge;
	cart->PPUPeakCartridge = m000PPUReadCartridge;
	cart->PPUWriteCartridge = m000PPUWriteCartridge;

	cart->PPUMirrorNametable = m000PPUMirrorNametable;

	Mapper000* map = malloc(sizeof(Mapper000));
	assert(map);
	memset(map, 0, sizeof(Mapper000));
	cart->mapper = map;

	map->chr_is_ram = chr_is_ram(header);

	// Skip trainer if present
	if (header->Trainer)
	{
		fseek(file, 512, SEEK_CUR);
	}

	map->PRG_ROM_Banks = num_prg_banks(header);

	if (map->PRG_ROM_Banks == 1)
	{
		map->PRG_ROM = malloc(16 * 1024);
		fread(map->PRG_ROM, 16 * 1024, 1, file);
	}
	else if (map->PRG_ROM_Banks == 2)
	{
		map->PRG_ROM = malloc(32 * 1024);
		fread(map->PRG_ROM, 32 * 1024, 1, file);
	}
	else
	{
		printf("[ERROR] Invalid PRG ROM size\n");
	}

	// Read CHR ROM into cartridge
	fread(map->CHR, 8 * 1024, 1, file);

	// Set Nametable mirroring mode
	map->mirrorMode = header->MirrorType == 0 ? HORIZONTAL : VERTICAL;

	// Bind Pattern table to renderer
	RendererSetPatternTable(map->CHR, 0);
	RendererSetPatternTable(map->CHR + 0x1000, 1);
}
