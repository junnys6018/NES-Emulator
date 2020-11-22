#include "2C02_Bus.h"

uint8_t ppu_bus_read(Bus2C02* bus, uint16_t addr)
{
	addr &= 0x3FFF; // Just in case given address is outside the PPU's addressable range

	// Read CHR from cartridge
	if (addr >= 0x0000 && addr < 0x2000) 
	{
		return bus->cartridge->PPUReadCartridge(bus->cartridge->mapper, addr);
	}
	// Read Nametable
	else if (addr >= 0x2000 && addr < 0x3F00)
	{
		// Mirror into $2000-$2FFF address range
		addr = 0x2000 + (addr & 0x0FFF);
		Cartridge* c = bus->cartridge;
		NametableIndex idx = c->PPUMirrorNametable(c->mapper, addr);

		return bus->nametable[idx.index][idx.addr];
	}
	// Read Palette data
	else if (addr >= 0x3F00 && addr < 0x4000)
	{
		// Mirror every 32 bytes
		addr &= 0x1F;
		return bus->palette[addr];
	}
	return 0;
}

uint8_t ppu_bus_peek(Bus2C02* bus, uint16_t addr)
{
	addr &= 0x3FFF;
	if (addr >= 0x0000 && addr < 0x2000)
	{
		return bus->cartridge->PPUPeakCartridge(bus->cartridge->mapper, addr);
	}
	else
	{
		return ppu_bus_read(bus, addr);
	}
}

void ppu_bus_write(Bus2C02* bus, uint16_t addr, uint8_t data)
{
	addr &= 0x3FFF; // Just in case given address is outside the PPU's addressable range

	// Write data to cartridge
	if (addr >= 0x0000 && addr < 0x2000)
	{
		bus->cartridge->PPUWriteCartridge(bus->cartridge->mapper, addr, data);
	}
	// Write to Nametable
	else if (addr >= 0x2000 && addr < 0x3F00)
	{
		// Mirror into $2000-$2FFF address range
		addr = 0x2000 + (addr & 0x0FFF);
		Cartridge* c = bus->cartridge;
		NametableIndex idx = c->PPUMirrorNametable(c->mapper, addr);

		bus->nametable[idx.index][idx.addr] = data;
	}
	// Write to Palette
	else if (addr >= 0x3F00 && addr < 0x4000)
	{
		// Mirror every 32 bytes
		addr &= 0x1F;
		bus->palette[addr] = data;

		// Addresses $3F10/$3F14/$3F18/$3F1C are mirrors of $3F00/$3F04/$3F08/$3F0C
		if ((addr & 0x3) == 0x00)
		{
			// Flip bit	4 of addr
			addr ^= 0x10;
			bus->palette[addr] = data;
		}
	}
}
