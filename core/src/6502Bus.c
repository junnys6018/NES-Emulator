#include "6502Bus.h"
#include "2C02.h"
#include "6502.h"
#include <stdio.h> // For printf

void cpu_bus_write(Bus6502* bus, uint16_t addr, uint8_t data)
{
	// Check if cartridge wants to handle the write
	bool wrote;
	Cartridge* cart = bus->cartridge;
	cart->cpu_write_cartridge(cart, addr, data, &wrote);
	if (!wrote)
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
			ppu_write(bus->ppu, addr, data);
		}
		// NES APU
		else if ((addr >= 0x4000 && addr < 0x4014) || addr == 0x4015 || addr == 0x4017)
		{
			apu_write(bus->apu, addr, data);
		}
		// DMA Port
		else if (addr == 0x4014)
		{
			bus->cpu->OAMDMA = data;
			bus->cpu->dma_transfer_cycles = ((bus->cpu->total_cycles % 2 == 1) ? 514 : 513);
		}
		// I/O registers
		else if (addr == 0x4016)
		{
			gamepad_write(bus->pad, data);
		}
		// APU and I/O functionality that is normally disabled
		else if (addr >= 0x4018 && addr < 0x4020)
		{

		}
	}
}

uint8_t cpu_bus_read(Bus6502* bus, uint16_t addr)
{
	// Check if cartridge wants to handle the read
	bool read;
	Cartridge* cart = bus->cartridge;
	uint8_t ret = cart->cpu_read_cartridge(cart, addr, &read);
	if (!read)
	{
		// Read internal RAM
		if (addr >= 0x0000 && addr < 0x2000)
		{
			addr &= 0x07FF;
			ret = bus->memory[addr];
		}
		// Read PPU registers
		else if (addr >= 0x2000 && addr < 0x4000)
		{
			// Mirror every 8 bytes
			addr = 0x2000 + (addr & 0x07);
			ret = ppu_read(bus->ppu, addr);
		}
		// APU interrupt and length counter
		else if (addr == 0x4015)
		{
			ret = apu_read(bus->apu, addr);
		}
		// I/O registers
		else if (addr == 0x4016)
		{
			ret = gamepad_read(bus->pad);
		}
		// APU and I/O functionality that is normally disabled
		else if (addr >= 0x4018 && addr < 0x4020)
		{

		}
	}

	uint8_t cheat_read_byte = cheat_code_read_system(&bus->cheats, addr, ret, &read);
	if (read)
		return cheat_read_byte;

	return ret;
}
