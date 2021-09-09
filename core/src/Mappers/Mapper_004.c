#include "Mapper_004.h"
#include "MapperUtils.h"

#include <stdlib.h>
#include <string.h>

// TODO: four screen

void clock_irq(Mapper004* map)
{
	if (map->IRQ_counter == 0 || map->IRQ_reload)
	{
		map->IRQ_reload = false;
		map->IRQ_counter = map->IRQ_latch;
	}
	else 
	{
		map->IRQ_counter--;
	}

	if (map->IRQ_counter == 0 && map->IRQ_enable)
	{
		irq_set(map->cpu, 2);
	}
}

uint8_t m004_cpu_read_cartridge(Cartridge* cart, uint16_t addr, bool* read)
{
	*read = (addr >= 0x4020 && addr <= 0xFFFF);
	if (!*read)
		return 0;

	Mapper004* map = (Mapper004*)cart->mapper;
	if (addr >= 0x6000 && addr < 0x8000)
	{
		if (map->ram_protect.bits.R)
		{
			return map->PRG_RAM[addr & 0x1FFF];
		}
		else
		{
			return 0; // ment to be open bus
		}
	}

	uint32_t bank_index = 0;

	// 8K switchable bank0
	if ((addr >= 0x8000 && addr < 0xA000 && !map->bank_select.bits.P) || (addr >= 0xC000 && addr < 0xE000 && map->bank_select.bits.P))
	{
		bank_index = map->PRG_bank0_select;
	}
	// 8K switchable bank1
	else if (addr >= 0xA000 && addr < 0xC000)
	{
		bank_index = map->PRG_bank1_select;
	}
	// Fixed to second last bank
	else if ((addr >= 0x8000 && addr < 0xA000 && map->bank_select.bits.P) || (addr >= 0xC000 && addr < 0xE000 && !map->bank_select.bits.P)) 
	{
		bank_index = map->PRG_ROM_banks - 2;
	}
	// Fixed to last bank
	else if (addr >= 0xE000 && addr <= 0xFFFF)
	{
		bank_index = map->PRG_ROM_banks - 1;
	}
	return map->PRG_ROM[(bank_index << 13) | (addr & 0x1FFF)];
}

void m004_cpu_write_cartridge(Cartridge* cart, uint16_t addr, uint8_t data, bool* wrote)
{
	*wrote = (addr >= 0x4020 && addr <= 0xFFFF);

	Mapper004* map = (Mapper004*)cart->mapper;

	// Write to ram
	if (addr >= 0x6000 && addr < 0x8000)
	{
		if (!map->ram_protect.bits.W && map->ram_protect.bits.R)
		{
			map->PRG_RAM[addr & 0x1FFF] = data;
		}
	}
	else if (addr >= 0x8000 && addr < 0xA000 && addr % 2 == 0)
	{
		map->bank_select.reg = data;
		if (map->bank_select.bits.C)
		{
			cart->updatePatternTableCB(map->left_pt, 1);
			cart->updatePatternTableCB(map->right_pt, 0);
		}
		else
		{
			cart->updatePatternTableCB(map->left_pt, 0);
			cart->updatePatternTableCB(map->right_pt, 1);
		}
	}
	else if (addr >= 0x8000 && addr < 0xA000 && addr % 2 == 1)
	{
		switch (map->bank_select.bits.R)
		{
		case 0:
			map->CHR_bank0_select = data % map->CHR_banks;
			memcpy(&map->left_pt[0], &map->CHR_ROM[(map->CHR_bank0_select & 0xFE) << 10], 0x800);
			break;
		case 1:
			map->CHR_bank1_select = data % map->CHR_banks;
			memcpy(&map->left_pt[0x800], &map->CHR_ROM[(map->CHR_bank1_select & 0xFE) << 10], 0x800);
			break;
		case 2:
			map->CHR_bank2_select = data % map->CHR_banks;
			memcpy(&map->right_pt[0], &map->CHR_ROM[map->CHR_bank2_select << 10], 0x400);
			break;
		case 3:
			map->CHR_bank3_select = data % map->CHR_banks;
			memcpy(&map->right_pt[0x400], &map->CHR_ROM[map->CHR_bank3_select << 10], 0x400);
			break;
		case 4:
			map->CHR_bank4_select = data % map->CHR_banks;
			memcpy(&map->right_pt[0x800], &map->CHR_ROM[map->CHR_bank4_select << 10], 0x400);
			break;
		case 5:
			map->CHR_bank5_select = data % map->CHR_banks;
			memcpy(&map->right_pt[0xC00], &map->CHR_ROM[map->CHR_bank5_select << 10], 0x400);
			break;
		case 6:
			map->PRG_bank0_select = data % map->PRG_ROM_banks;
			break;
		case 7:
			map->PRG_bank1_select = data % map->PRG_ROM_banks;
			break;
		}
	}
	else if (addr >= 0xA000 && addr < 0xC000 && addr % 2 == 0)
	{
		map->mirroring = data & 1;
	}
	else if (addr >= 0xA000 && addr < 0xC000 && addr % 2 == 1)
	{
		map->ram_protect.reg = data;
	}
	else if (addr >= 0xC000 && addr < 0xE000 && addr % 2 == 0)
	{
		map->IRQ_latch = data;
	}
	else if (addr >= 0xC000 && addr < 0xE000 && addr % 2 == 1)
	{
		map->IRQ_reload = true;
	}
	else if (addr >= 0xE000 && addr <= 0xFFFF)
	{
		map->IRQ_enable = addr & 1;
		if (!map->IRQ_enable)
		{
			irq_clear(map->cpu, 2);
		}
	}
}

uint8_t m004_ppu_peek_cartridge(Cartridge* cart, uint16_t addr)
{
	Mapper004* map = (Mapper004*)cart->mapper;

	// CHR A12 invert
	if (map->bank_select.bits.C)
	{
		addr = addr ^ 0x1000; // Invert bit 12
	}

	uint32_t bank_index = 0;
	if (addr >= 0x0000 && addr < 0x0400)
	{
		bank_index = map->CHR_bank0_select & 0xFE; // Clear bit 0
	}
	else if (addr >= 0x0400 && addr < 0x0800)
	{
		bank_index = map->CHR_bank0_select | 0x01; // Set bit 0
	}
	else if (addr >= 0x0800 && addr < 0x0C00)
	{
		bank_index = map->CHR_bank1_select & 0xFE;
	}
	else if (addr >= 0x0C00 && addr < 0x1000)
	{
		bank_index = map->CHR_bank1_select | 0x01;
	}
	else if (addr >= 0x1000 && addr < 0x1400)
	{
		bank_index = map->CHR_bank2_select;
	}
	else if (addr >= 0x1400 && addr < 0x1800)
	{
		bank_index = map->CHR_bank3_select;
	}
	else if (addr >= 0x1800 && addr < 0x1C00)
	{
		bank_index = map->CHR_bank4_select;
	}
	else if (addr >= 0x1C00 && addr <= 0x1FFF)
	{
		bank_index = map->CHR_bank5_select;
	}

	return map->CHR_ROM[(bank_index << 10) | (addr & 0x03FF)];
}

uint8_t m004PPUReadCartridge(Cartridge* cart, uint16_t addr)
{
	Mapper004* map = (Mapper004*)cart->mapper;

	// PPU A12 rising edge
	bool PPU_A12 = (addr & 0x1000) > 0;
	if (!map->old_PPU_A12 && PPU_A12)
	{
		clock_irq(map);
	}
	map->old_PPU_A12 = PPU_A12;

	return m004_ppu_peek_cartridge(cart, addr);
}

void m004_ppu_write_cartridge(Cartridge* cart, uint16_t addr, uint8_t data)
{
	// Do nothing, CHR is a rom
}

NametableIndex m004_ppu_mirror_nametable(void* mapper, uint16_t addr)
{
	Mapper004* map = (Mapper004*)mapper;
	if (map->mirroring)
	{
		return mirror_horizontal(addr);
	}
	else
	{
		return mirror_vertical(addr);
	}
}

void m004_free(Mapper004* mapper)
{
	Mapper004* map = (Mapper004*)mapper;
	free(map->PRG_ROM);
	free(map->CHR_ROM);
	free(mapper);
}

void m004_load_from_file(Header* header, Cartridge* cart, FILE* file, State6502* cpu)
{
	if (header->FourScreen)
	{
		printf("[ERROR] Not implemented 4-screen mirroring on mapper 4\n");
		return;
	}

	cart->CPUReadCartridge = m004_cpu_read_cartridge;
	cart->CPUWriteCartridge = m004_cpu_write_cartridge;

	cart->PPUReadCartridge = m004PPUReadCartridge;
	cart->PPUPeakCartridge = m004_ppu_peek_cartridge;
	cart->PPUWriteCartridge = m004_ppu_write_cartridge;

	cart->PPUMirrorNametable = m004_ppu_mirror_nametable;

	Mapper004* map = malloc(sizeof(Mapper004));
	assert(map);
	memset(map, 0, sizeof(Mapper004));
	cart->mapper = map;
	map->cpu = cpu;

	// Skip trainer if present
	if (header->Trainer)
	{
		fseek(file, 512, SEEK_CUR);
	}

	if (num_prg_banks(header) > 32 || num_chr_banks(header) > 32)
	{
		printf("[ERROR] Too many banks\n");	
	}

	map->PRG_ROM_banks = 2 * num_prg_banks(header); // Convert from 16K banks to 8K banks
	map->CHR_banks =  8 * num_chr_banks(header); // Convert from 8K banks to 1K banks

	map->PRG_ROM = malloc((size_t)map->PRG_ROM_banks * 8 * 1024);
	map->CHR_ROM = malloc((size_t)map->CHR_banks * 1024);

	fread(map->PRG_ROM, (size_t)map->PRG_ROM_banks * 8 * 1024, 1, file);
	fread(map->CHR_ROM, (size_t)map->CHR_banks * 1024, 1, file);

	if (cart->updatePatternTableCB)
	{
		cart->updatePatternTableCB(map->left_pt, 0);
		cart->updatePatternTableCB(map->right_pt, 1);
	}
}