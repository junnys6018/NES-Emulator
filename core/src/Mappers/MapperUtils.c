#include "MapperUtils.h"

NametableIndex mirror_vertical(uint16_t addr)
{
	NametableIndex ret;
	ret.addr = addr & 0x3FF;

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

	return ret;
}

NametableIndex mirror_horizontal(uint16_t addr)
{
	NametableIndex ret;
	ret.addr = addr & 0x3FF;

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

	return ret;
}

NametableIndex mirror_one_screen_lower(uint16_t addr)
{
	NametableIndex ret;
	ret.addr = addr & 0x3FF;
	ret.index = 0;
	return ret;
}

NametableIndex mirror_one_screen_upper(uint16_t addr)
{
	NametableIndex ret;
	ret.addr = addr & 0x3FF;
	ret.index = 1;
	return ret;
}
