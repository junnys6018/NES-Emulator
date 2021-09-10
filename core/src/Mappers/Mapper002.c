#include "Mapper002.h"

uint8_t m002_cpu_read_cartridge(Cartridge* cart, uint16_t addr, bool* read)
{
	*read = (addr >= 0x4020 && addr <= 0xFFFF);

	Mapper002* map002 = (Mapper002*)cart->mapper;
	if (addr >= 0x8000 && addr < 0xC000)
	{
		// Bankswitch
		uint32_t index = ((uint32_t)map002->PRG_bank_select << 14) | (addr & 0x3FFF);
		return map002->PRG_ROM[index];
	}
	else if (addr >= 0xC000 && addr <= 0xFFFF)
	{
		// Fix to last bank
		return map002->PRG_ROM[((uint32_t)(map002->PRG_ROM_banks - 1) << 14) | (addr & 0x3FFF)];
	}

	return 0;
}

void m002_cpu_write_cartridge(Cartridge* cart, uint16_t addr, uint8_t data, bool* wrote)
{
	*wrote = (addr >= 0x4020 && addr <= 0xFFFF);

	Mapper002* map002 = (Mapper002*)cart->mapper;
	if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		map002->PRG_bank_select = data % map002->PRG_ROM_banks;
	}
}

uint8_t m002_ppu_read_cartridge(Cartridge* cart, uint16_t addr)
{
	Mapper002* map002 = (Mapper002*)cart->mapper;
	return map002->CHR[addr];
}

void m002_ppu_write_cartridge(Cartridge* cart, uint16_t addr, uint8_t data)
{
	Mapper002* map002 = (Mapper002*)cart->mapper;
	if (map002->chr_is_ram)
	{
		map002->CHR[addr] = data;
	}
}

NametableIndex m002_ppu_mirror_nametable(void* mapper, uint16_t addr)
{
	Mapper002* map002 = (Mapper002*)mapper;

	switch (map002->mirror_mode)
	{
	case HORIZONTAL:
		return mirror_horizontal(addr);
	case VERTICAL:
		return mirror_vertical(addr);
	default:
		printf("[Error] Unreachable code at Mapper_002.c\n");
		break;
	}
}

void m002_free(Mapper002* mapper)
{
	free(mapper->PRG_ROM);
	free(mapper);
}

void m002_load_from_file(Header* header, Cartridge* cart, FILE* file)
{
	cart->cpu_read_cartridge = m002_cpu_read_cartridge;
	cart->cpu_write_cartridge = m002_cpu_write_cartridge;

	cart->ppu_read_cartridge = m002_ppu_read_cartridge;
	cart->ppu_peak_cartridge = m002_ppu_read_cartridge;
	cart->ppu_write_cartridge = m002_ppu_write_cartridge;

	cart->ppu_mirror_nametable = m002_ppu_mirror_nametable;

	Mapper002* map = malloc(sizeof(Mapper002));
	assert(map);
	memset(map, 0, sizeof(Mapper002));
	cart->mapper = map;

	// Skip trainer if present
	if (header->trainer)
	{
		fseek(file, 512, SEEK_CUR);
	}

	map->PRG_ROM_banks = num_prg_banks(header);;
	map->chr_is_ram = chr_is_ram(header);


	map->PRG_ROM = malloc((size_t)map->PRG_ROM_banks * 16 * 1024);

	fread(map->PRG_ROM, (size_t)map->PRG_ROM_banks * 16 * 1024, 1, file);
	if (!map->chr_is_ram)
		fread(map->CHR, 8 * 1024, 1, file);

	// Set the mirror mode
	map->mirror_mode = header->mirror_type == 1 ? VERTICAL : HORIZONTAL;

	// Bind Pattern table to renderer
	if (cart->update_pattern_table_cb)
	{
		cart->update_pattern_table_cb(map->CHR, 0);
		cart->update_pattern_table_cb(map->CHR + 0x1000, 1);
	}

}
