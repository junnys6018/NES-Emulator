#include "nes.h"

void NESInit(Nes* nes, const char* filepath)
{
	load_cartridge_from_file(&nes->cart, filepath);

	nes->cpu_bus.cartridge = &nes->cart;
	nes->cpu_bus.ppu = &nes->ppu;
	nes->cpu_bus.cpu = &nes->cpu;
	nes->cpu_bus.pad = &nes->pad;

	nes->ppu_bus.cartridge = &nes->cart;

	nes->cpu.bus = &nes->cpu_bus;

	nes->ppu.bus = &nes->ppu_bus;
	nes->ppu.cpu = &nes->cpu;

	power_on_6502(&nes->cpu);
	reset_6502(&nes->cpu);

	power_on_2C02(&nes->ppu);
	reset_2C02(&nes->ppu);

	long long clocks = 0;
}

void NESDestroy(Nes* nes)
{
	free_cartridge(&nes->cart);
}

void NESReset(Nes* nes)
{
	nes->system_clock = 0;

	power_on_6502(&nes->cpu);
	reset_6502(&nes->cpu);

	power_on_2C02(&nes->ppu);
	reset_2C02(&nes->ppu);
}
