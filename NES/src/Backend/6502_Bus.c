#include "6502_Bus.h"
#include "2C02.h"

void cpu_bus_write(Bus6502* bus, uint16_t addr, uint8_t data)
{
	// Write to internal RAM
	if (addr >= 0x0000 && addr < 0x2000)
	{
		addr &= 0x07FF;
		bus->memory[addr] = data;
	}
	// Write to PPU registers
	else if (addr >= 0x2000 && addr < 0x4000)
	{
		// Mirror every 8 bytes
		addr = 0x2000 + (addr & 0x07);
		write_ppu(bus->ppu, addr, data);
	}
	// NES APU and I/O registers
	else if (addr >= 0x4000 && addr < 0x4018)
	{

	}
	// APU and I/O functionality that is normally disabled
	else if (addr >= 0x4018 && addr < 0x4020)
	{

	}
	// Cartridge
	else if (addr >= 0x4020 && addr <= 0xFFFF)
	{
		Cartridge* cart = bus->cartridge;
		cart->CPUWriteCartridge(cart->mapper, addr, data);
	}
}

uint8_t cpu_bus_read(Bus6502* bus, uint16_t addr)
{
	// Read internal RAM
	if (addr >= 0x0000 && addr < 0x2000)
	{
		addr &= 0x07FF;
		return bus->memory[addr];
	}
	// Read PPU registers
	else if (addr >= 0x2000 && addr < 0x4000)
	{
		// Mirror every 8 bytes
		addr = 0x2000 + (addr & 0x07);
		return read_ppu(bus->ppu, addr);
	}
	// NES APU and I/O registers
	else if (addr >= 0x4000 && addr < 0x4018)
	{

	}
	// APU and I/O functionality that is normally disabled
	else if (addr >= 0x4018 && addr < 0x4020)
	{

	}
	// Cartridge
	else if (addr >= 0x4020 && addr <= 0xFFFF)
	{
		Cartridge* cart = bus->cartridge;
		return cart->CPUReadCartridge(cart->mapper, addr);
	}
	return 0;
}
