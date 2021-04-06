#include "Mapper_002.h"

uint8_t m002CPUReadCartridge(void* mapper, uint16_t addr, bool* read)
{
	*read = (addr >= 0x4020 && addr <= 0xFFFF);

	Mapper002* map002 = (Mapper002*)mapper;
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

void m002CPUWriteCartridge(void* mapper, uint16_t addr, uint8_t data, bool* wrote)
{
	*wrote = (addr >= 0x4020 && addr <= 0xFFFF);

	Mapper002* map002 = (Mapper002*)mapper;
	if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		map002->PRG_bank_select = data % map002->PRG_ROM_banks;
	}
}

uint8_t m002PPUReadCartridge(void* mapper, uint16_t addr)
{
	Mapper002* map002 = (Mapper002*)mapper;
	return map002->CHR[addr];
}

void m002PPUWriteCartridge(void* mapper, uint16_t addr, uint8_t data)
{
	Mapper002* map002 = (Mapper002*)mapper;
	if (map002->chr_is_ram)
	{
		map002->CHR[addr] = data;
	}
}

NametableIndex m002PPUMirrorNametable(void* mapper, uint16_t addr)
{
	Mapper002* map002 = (Mapper002*)mapper;

	switch (map002->mirrorMode)
	{
	case HORIZONTAL:
		return MirrorHorizontal(addr);
	case VERTICAL:
		return MirrorVertical(addr);
	default:
		printf("[Error] Unreachable code at Mapper_002.c\n");
		break;
	}
}

void m002Free(Mapper002* mapper)
{
	free(mapper->PRG_ROM);
	free(mapper);
}

void m002LoadFromFile(Header* header, Cartridge* cart, FILE* file)
{
	cart->CPUReadCartridge = m002CPUReadCartridge;
	cart->CPUWriteCartridge = m002CPUWriteCartridge;

	cart->PPUReadCartridge = m002PPUReadCartridge;
	cart->PPUPeakCartridge = m002PPUReadCartridge;
	cart->PPUWriteCartridge = m002PPUWriteCartridge;

	cart->PPUMirrorNametable = m002PPUMirrorNametable;

	Mapper002* map = malloc(sizeof(Mapper002));
	assert(map);
	memset(map, 0, sizeof(Mapper002));
	cart->mapper = map;

	// Skip trainer if present
	if (header->Trainer)
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
	map->mirrorMode = header->MirrorType == 1 ? VERTICAL : HORIZONTAL;

	// Bind Pattern table to renderer
	ControllerSetPatternTable(map->CHR, 0);
	ControllerSetPatternTable(map->CHR + 0x1000, 1);
}
