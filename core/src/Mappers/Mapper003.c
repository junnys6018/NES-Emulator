#include "Mapper003.h"
#include <string.h>

uint8_t m003_cpu_read_cartridge(Cartridge* cart, uint16_t addr, bool* read)
{
	*read = (addr >= 0x4020 && addr <= 0xFFFF);

	Mapper003* map003 = (Mapper003*)cart->mapper;
	if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		addr &= (map003->PRG_ROM_banks == 1 ? 0x3FFF : 0x7FFF);
		return map003->PRG_ROM[addr];
	}

	return 0;
}

void m003_cpu_write_cartridge(Cartridge* cart, uint16_t addr, uint8_t data, bool* wrote)
{
	*wrote = (addr >= 0x4020 && addr <= 0xFFFF);

	Mapper003* map003 = (Mapper003*)cart->mapper;
	if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		map003->CHR_bank_select = data % map003->CHR_banks;
		uint32_t index = (uint32_t)(map003->CHR_bank_select << 13);
		if (cart->updatePatternTableCB)
		{
			cart->updatePatternTableCB(map003->CHR + index, 0);
			cart->updatePatternTableCB(map003->CHR + index + 0x1000, 1);
		}
	}
}

uint8_t m003_ppu_read_cartridge(Cartridge* cart, uint16_t addr)
{
	Mapper003* map003 = (Mapper003*)cart->mapper;

	uint32_t index = ((uint32_t)map003->CHR_bank_select << 13) | addr;
	return map003->CHR[index];
}

void m003_ppu_write_cartridge(Cartridge* cart, uint16_t addr, uint8_t data)
{
	Mapper003* map003 = (Mapper003*)cart->mapper;
	if (map003->chr_is_ram)
	{
		uint32_t index = ((uint32_t)map003->CHR_bank_select << 13) | addr;
		map003->CHR[index] = data;
	}
}

NametableIndex m003_ppu_mirror_nametable(void* mapper, uint16_t addr)
{
	Mapper003* map003 = (Mapper003*)mapper;
	switch (map003->mirrorMode)
	{
	case HORIZONTAL:
		return mirror_horizontal(addr);
	case VERTICAL:
		return mirror_vertical(addr);
	}
}

void m003_free(Mapper003* mapper)
{
	free(mapper->PRG_ROM);
	free(mapper->CHR);
	free(mapper);
}

void m003_load_from_file(Header* header, Cartridge* cart, FILE* file)
{
	cart->CPUReadCartridge = m003_cpu_read_cartridge;
	cart->CPUWriteCartridge = m003_cpu_write_cartridge;

	cart->PPUReadCartridge = m003_ppu_read_cartridge;
	cart->PPUPeakCartridge = m003_ppu_read_cartridge;
	cart->PPUWriteCartridge = m003_ppu_write_cartridge;

	cart->PPUMirrorNametable = m003_ppu_mirror_nametable;

	Mapper003* map = malloc(sizeof(Mapper003));
	assert(map);
	memset(map, 0, sizeof(Mapper003));
	cart->mapper = map;

	// Skip trainer if present
	if (header->Trainer)
	{
		fseek(file, 512, SEEK_CUR);
	}

	map->PRG_ROM_banks = num_prg_banks(header);
	assert(map->PRG_ROM_banks == 1 || map->PRG_ROM_banks == 2);

	map->CHR_banks = num_chr_banks(header);
	if (chr_is_ram(header))
	{
		map->chr_is_ram = true;
		map->CHR_banks = 1; // CHR is a RAM
	}
	else
	{
		map->chr_is_ram = false;
	}

	map->PRG_ROM = malloc((size_t)map->PRG_ROM_banks * 16 * 1024);
	map->CHR = malloc((size_t)map->CHR_banks * 8 * 1024);

	fread(map->PRG_ROM, (size_t)map->PRG_ROM_banks * 16 * 1024, 1, file);
	fread(map->CHR, (size_t)map->CHR_banks * 8 * 1024, 1, file);

	// Set the mirror mode
	map->mirrorMode = header->MirrorType == 1 ? VERTICAL : HORIZONTAL;
}
