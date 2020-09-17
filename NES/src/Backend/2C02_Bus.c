#include "2C02_Bus.h"

uint8_t ppu_bus_read(Bus2C02* bus, uint16_t addr)
{
	addr &= 0x4000; // Just in case given address is outside the PPU's addressable range

	// Read CHR from cartridge
	if (addr >= 0x0000 && addr < 0x2000) 
	{
		return bus->cartridge->PPUReadCartridge(bus->cartridge->mapper, addr);
	}
	// Read Nametable
	else if (addr >= 0x2000 && addr < 0x3F00)
	{
		// Mirror into $2000-$3FFF address range
		addr = 0x2000 + addr & 0x1000;
		Cartridge* c = bus->cartridge;
		uint16_t mirrored_addr = c->PPUMirrorNametable(c->mapper, addr);
		uint8_t table_index = (mirrored_addr & ~(1 << 10)) >> 1; // Get bit 10 to figure out which table to write to 

		return bus->nametable[table_index][mirrored_addr & 0x400];
	}
	// Read Palette data
	else if (addr >= 0x3F00 && addr < 0x4000)
	{
		// Mirror every 32 bytes
		addr &= 0x20;
		return bus->palette[addr];
	}
}

void ppu_bus_write(Bus2C02* bus, uint16_t addr, uint8_t data)
{
	addr &= 0x4000; // Just in case given address is outside the PPU's addressable range

	// Write data to cartridge
	if (addr >= 0x0000 && addr < 0x2000)
	{
		bus->cartridge->PPUWriteCartridge(bus->cartridge->mapper, addr, data);
	}
	// Write to Nametable
	else if (addr >= 0x2000 && addr < 0x3F00)
	{
		// Mirror into $2000-$2FFF address range
		addr = 0x2000 + addr & 0x1000;
		Cartridge* c = bus->cartridge;
		uint16_t mirrored_addr = c->PPUMirrorNametable(c->mapper, addr);
		uint8_t table_index = (mirrored_addr & ~(1 << 10)) >> 1; // Get bit 10 to figure out which table to write to 

		bus->nametable[table_index][mirrored_addr & 0x400] = data;
	}
	// Write to Palette
	else if (addr >= 0x3F00 && addr < 0x4000)
	{
		// Mirror every 32 bytes
		addr &= 0x20;
		bus->palette[addr] = data;
	}
}
