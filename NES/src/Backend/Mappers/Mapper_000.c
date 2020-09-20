#include "Mapper_000.h"
#include <stdlib.h>

uint8_t m000CPUReadCartridge(void* mapper, uint16_t addr)
{
	Mapper000* map000 = (Mapper000*)mapper;
	// Read from PGR_RAM
	if (addr >= 0x6000 && addr < 0x8000)
	{
		addr &= 0x1FFF;
		return map000->PRG_RAM[addr];
	}
	// Read from PGR_ROM
	else if (addr >= 0x8000 && addr < 0xFFFF)
	{
		addr &= (map000->romCap == _16K ? 0x3FFF : 0x7FFF);
		return map000->PRG_ROM[addr];
	}
	return 0;
}

uint8_t m000PPUReadCartridge(void* mapper, uint16_t addr)
{
	Mapper000* map000 = (Mapper000*)mapper;
	return map000->CHR[addr];
}

void m000CPUWriteCartridge(void* mapper, uint16_t addr, uint8_t data)
{
	Mapper000* map000 = (Mapper000*)mapper;
	// Write to PGR_RAM
	if (addr >= 0x6000 && addr < 0x8000)
	{
		addr &= 0x1FFF;
		map000->PRG_RAM[addr] = data;
	}
}

void m000PPUWriteCartridge(void* mapper, uint16_t addr, uint8_t data)
{
	Mapper000* map000 = (Mapper000*)mapper;
	map000->CHR[addr] = data;
}

NametableIndex m000PPUMirrorNametable(void* mapper, uint16_t addr)
{
	NametableIndex ret;
	ret.addr = addr & 0x3FF;

	Mapper000* map000 = (Mapper000*)mapper;
	if (map000->mirrorMode == HORIZONTAL)
	{
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
	}
	else if (map000->mirrorMode == VERTICAL)
	{
		if (addr >= 0x2000 && addr < 0x2400 || addr >= 0x2800 && addr < 0x2C00)
		{
			// Map into first table
			ret.index = 0;
		}
		else
		{
			// Map into second table
			ret.index = 1;
		}
	}

	return ret;
}

void m000Free(Mapper000* mapper)
{
	free(mapper->PRG_ROM);
	free(mapper);
}
