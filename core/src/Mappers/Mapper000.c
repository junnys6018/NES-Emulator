#include "Mapper000.h"
#include <stdlib.h>
#include <string.h>

uint8_t m000_cpu_read_cartridge(Cartridge* cart, uint16_t addr, bool* read)
{
	*read = (addr >= 0x4020 && addr <= 0xFFFF);

	Mapper000* map000 = (Mapper000*)cart->mapper;
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

void m000_cpu_write_cartridge(Cartridge* cart, uint16_t addr, uint8_t data, bool* wrote)
{
	*wrote = (addr >= 0x4020 && addr <= 0xFFFF);

	Mapper000* map000 = (Mapper000*)cart->mapper;
	// Write to PGR_RAM
	if (addr >= 0x6000 && addr < 0x8000)
	{
		addr &= 0x1FFF;
		map000->PRG_RAM[addr] = data;
	}
}

uint8_t m000_ppu_read_cartridge(Cartridge* cart, uint16_t addr)
{
	Mapper000* map000 = (Mapper000*)cart->mapper;
	return map000->CHR[addr];
}

void m000_ppu_write_cartridge(Cartridge* cart, uint16_t addr, uint8_t data)
{
	Mapper000* map000 = (Mapper000*)cart->mapper;
	if (map000->chr_is_ram)
	{
		map000->CHR[addr] = data;
	}
}

NametableIndex m000_ppu_mirror_nametable(void* mapper, uint16_t addr)
{
	Mapper000* map000 = (Mapper000*)mapper;
	switch (map000->mirrorMode)
	{
	case HORIZONTAL:
		return mirror_horizontal(addr);
	case VERTICAL:
		return mirror_vertical(addr);
	}
}

void m000_free(Mapper000* mapper)
{
	free(mapper->PRG_ROM);
	free(mapper);
}

void m000_load_from_file(Header* header, Cartridge* cart, FILE* file)
{
	cart->CPUReadCartridge = m000_cpu_read_cartridge;
	cart->CPUWriteCartridge = m000_cpu_write_cartridge;

	cart->PPUReadCartridge = m000_ppu_read_cartridge;
	cart->PPUPeakCartridge = m000_ppu_read_cartridge;
	cart->PPUWriteCartridge = m000_ppu_write_cartridge;

	cart->PPUMirrorNametable = m000_ppu_mirror_nametable;

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
	if (cart->updatePatternTableCB)
	{
		cart->updatePatternTableCB(map->CHR, 0);
		cart->updatePatternTableCB(map->CHR + 0x1000, 1);
	}
}
