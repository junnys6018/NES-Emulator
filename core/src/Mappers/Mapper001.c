#include "Mapper001.h"
#include "MapperUtils.h"
#include <string.h>
#include <stdlib.h>

uint8_t m001_cpu_read_cartridge(Cartridge* cart, uint16_t addr, bool* read)
{
	*read = (addr >= 0x4020 && addr <= 0xFFFF);

	Mapper001* map001 = (Mapper001*)cart->mapper;
	if (addr >= 0x6000 && addr < 0x8000)
	{
		// PRG Ram chip enable (active low)
		if (map001->PRG_bank_select & 0x10)
		{
			return 0;
		}
		return map001->PRG_RAM[addr & 0x1FFF];
	}
	else if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		switch (map001->control.bits.P)
		{
		case 0: // switch 32KB at $8000
		case 1:
		{
			uint32_t bank_select = map001->PRG_bank_select & 0x0E;
			uint32_t index = (bank_select << 14) | (addr & 0x7FFF);
			return map001->PRG_ROM[index];
		}
		case 2: // fix first bank at $8000 and switch 16 KB bank at $C000
		{
			if (addr < 0xC000) // First bank
			{
				return map001->PRG_ROM[addr & 0x3FFF];
			}
			uint32_t bank_select = map001->PRG_bank_select & 0x0F;
			uint32_t index = (bank_select << 14) | (addr & 0x3FFF);
			return map001->PRG_ROM[index];
		}
		case 3: // fix last bank at $C000 and switch 16 KB bank at $8000
		{
			if (addr >= 0xC000) // Last bank
			{
				return map001->PRG_ROM[((uint32_t)(map001->PRG_ROM_banks - 1) << 14) | (addr & 0x3FFF)];
			}
			uint32_t bank_select = map001->PRG_bank_select & 0x0F;
			uint32_t index = (bank_select << 14) | (addr & 0x3FFF);
			return map001->PRG_ROM[index];
		}
		}
	}
	return 0;
}

void update_renderer_patter_table(Mapper001* mapper, UPDATE_PATTERN_TABLE_CB callback)
{
	if (mapper->control.bits.C)
	{
		callback(mapper->CHR + ((uint32_t)(mapper->CHR_bank0_select) << 12), 0);
		callback(mapper->CHR + ((uint32_t)(mapper->CHR_bank1_select) << 12), 1);
	}
	else
	{
		uint32_t base_addr = ((uint32_t)mapper->CHR_bank0_select >> 1) << 13;
		callback(mapper->CHR + base_addr, 0);
		callback(mapper->CHR + base_addr + 0x1000, 1);
	}
}

void m001_cpu_write_cartridge(Cartridge* cart, uint16_t addr, uint8_t data, bool* wrote)
{
	*wrote = (addr >= 0x4020 && addr <= 0xFFFF);

	Mapper001* map001 = (Mapper001*)cart->mapper;
	if (addr >= 0x6000 && addr < 0x8000)
	{
		// PRG Ram chip enable (active low)
		if (!(map001->PRG_bank_select & 0x10))
		{
			map001->PRG_RAM[addr & 0x1FFF] = data;
		}
	}
	else if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		if (data & 0x80)
		{
			map001->shift_register = 0b10000;
			map001->control.reg = map001->control.reg | 0x0C;
			return;
		}
		
		bool last_bit_high = map001->shift_register & 0x01;
		map001->shift_register = (map001->shift_register >> 1) | ((data & 0x01) << 4);
		if (last_bit_high)
		{
			bool update_pattern_table = false;
			uint8_t addr_bits_14_13 = (addr & 0x6000) >> 13;
			switch (addr_bits_14_13)
			{
			case 0:
				update_pattern_table = (map001->control.reg ^ map001->shift_register) & 0x10; // Update if the CHR_ROM bank mode has changed
				map001->control.reg = map001->shift_register;
				break;
			case 1:
				update_pattern_table = true;
				map001->CHR_bank0_select = map001->shift_register % (2 * map001->CHR_banks);
				break;
			case 2:
				update_pattern_table = map001->control.bits.C; // Only update if we are switching both banks
				map001->CHR_bank1_select = map001->shift_register % (2 * map001->CHR_banks);
				break;
			case 3:
				map001->PRG_bank_select = map001->shift_register % map001->PRG_ROM_banks;
				break;
			}
			map001->shift_register = 0b10000;

			if (update_pattern_table && cart->update_pattern_table_cb)
			{
				update_renderer_patter_table(map001, cart->update_pattern_table_cb);
			}
		}
	}
}

uint8_t m001_ppu_read_cartridge(Cartridge* cart, uint16_t addr)
{
	Mapper001* map001 = (Mapper001*)cart->mapper;
	switch (map001->control.bits.C)
	{
	case 0: // swtich 8KB at a time
	{
		uint32_t bank_select = map001->CHR_bank0_select & 0x1E;
		uint32_t index = (bank_select << 12) | addr;
		return map001->CHR[index];
	}
	case 1: // swtich 2x4KB banks independently
		if (addr < 0x1000)
		{
			uint32_t index = ((uint32_t)map001->CHR_bank0_select << 12) | (addr & 0x0FFF);
			return map001->CHR[index];
		}
		else
		{
			uint32_t index = ((uint32_t)map001->CHR_bank1_select << 12) | (addr & 0x0FFF);
			return map001->CHR[index];
		}
	}
	return 0;
}

void m001_ppu_write_cartridge(Cartridge* cart, uint16_t addr, uint8_t data)
{
	Mapper001* map001 = (Mapper001*)cart->mapper;
	switch (map001->control.bits.C)
	{
	case 0: // swtich 8KB at a time
	{
		uint32_t bank_select = map001->CHR_bank0_select & 0x1E;
		uint32_t index = (bank_select << 12) | addr;
		map001->CHR[index] = data;
		break;
	}
	case 1: // swtich 2x4KB banks independently
		if (addr < 0x1000)
		{
			uint32_t index = ((uint32_t)map001->CHR_bank0_select << 12) | (addr & 0x0FFF);
			map001->CHR[index] = data;
		}
		else
		{
			uint32_t index = ((uint32_t)map001->CHR_bank1_select << 12) | (addr & 0x0FFF);
			map001->CHR[index] = data;
		}
		break;
	}
}

NametableIndex m001_ppu_mirror_nametable(void* mapper, uint16_t addr)
{
	Mapper001* map001 = (Mapper001*)mapper;
	switch (map001->control.bits.M)
	{
	case ONE_SCREEN_LOWER:
		return mirror_one_screen_lower(addr);
	case ONE_SCREEN_UPPER: 
		return mirror_one_screen_upper(addr);
	case VERTICAL: 
		return mirror_vertical(addr);
	case HORIZONTAL:
		return mirror_horizontal(addr);
	default:
		printf("[ERROR] Unreachable code in mapper_001.c\n");
	}
}

void m001_free(Mapper001* mapper)
{
	free(mapper->PRG_ROM);
	free(mapper->PRG_RAM);
	free(mapper->CHR);
	free(mapper);
}

void m001_load_from_file(Header* header, Cartridge* cart, FILE* file)
{
	cart->cpu_read_cartridge = m001_cpu_read_cartridge;
	cart->cpu_write_cartridge = m001_cpu_write_cartridge;

	cart->ppu_read_cartridge = m001_ppu_read_cartridge;
	cart->ppu_peak_cartridge = m001_ppu_write_cartridge;
	cart->ppu_write_cartridge = m001_ppu_write_cartridge;

	cart->ppu_mirror_nametable = m001_ppu_mirror_nametable;

	Mapper001* map = malloc(sizeof(Mapper001));
	assert(map);
	memset(map, 0, sizeof(Mapper001));
	cart->mapper = map;

	// Power on in the last bank
	map->control.bits.P = 3;

	// Skip trainer if present
	if (header->trainer)
	{
		fseek(file, 512, SEEK_CUR);
	}

	map->PRG_ROM_banks = num_prg_banks(header);
	assert(map->PRG_ROM_banks <= 16); // Too many banks
	map->CHR_banks = num_chr_banks(header);
	if (chr_is_ram(header))
	{
		map->CHR_banks = 1; // CHR is a RAM
	}
	assert(map->CHR_banks <= 16); 

	map->PRG_RAM_banks = 1;

	map->PRG_RAM = malloc(8 * 1024);
	memset(map->PRG_RAM, 0, 8 * 1024);
	map->PRG_ROM = malloc((size_t)map->PRG_ROM_banks * 16 * 1024);
	map->CHR = malloc((size_t)map->CHR_banks * 8 * 1024);

	fread(map->PRG_ROM, (size_t)map->PRG_ROM_banks * 16 * 1024, 1, file);
	fread(map->CHR, (size_t)map->CHR_banks * 8 * 1024, 1, file);

	// Set the mirror mode
	map->control.bits.M = header->mirror_type == 1 ? VERTICAL : HORIZONTAL;
}

int m001_save_game(Cartridge* cart, FILE* savefile, char error_string[256])
{
	Mapper001* map = (Mapper001*)cart->mapper;
	fwrite(map->PRG_RAM, 8 * 1024, 1, savefile);
	return 0;
}

int m001_load_save(Cartridge* cart, FILE* savefile, char error_string[256])
{
	// get size of file
	fseek(savefile, 0, SEEK_END);
	long size = ftell(savefile);
	if (size != 8 * 1024)
	{
		if (error_string)
			sprintf(error_string, "invalid save file\n");
		return 1;
	}
	fseek(savefile, 0, SEEK_SET);

	Mapper001* map = (Mapper001*)cart->mapper;
	fread(map->PRG_RAM, 8 * 1024, 1, savefile);
	return 0;
}
