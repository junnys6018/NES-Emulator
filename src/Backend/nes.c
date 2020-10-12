#include "nes.h"
#include "Backend/Mappers/Mapper_JUN.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>

void NESInit(Nes* nes, const char* filepath)
{
	if (filepath)
	{
		load_cartridge_from_file(&nes->cart, filepath);
	}
	else // Load a dummy cart
	{
		memset(&nes->cart, 0, sizeof(nes->cart));
		nes->cart.mapperID = 767;

		nes->cart.CPUReadCartridge = mJUNCPUReadCartridge;
		nes->cart.PPUReadCartridge = mJUNPPUReadCartridge;

		nes->cart.CPUWriteCartridge = mJUNCPUWriteCartridge;
		nes->cart.PPUWriteCartridge = mJUNPPUWriteCartridge;

		nes->cart.PPUMirrorNametable = mJUNPPUMirrorNametable;

		MapperJUN* map = malloc(sizeof(MapperJUN));
		assert(map);
		memset(map, 0, sizeof(MapperJUN));
		nes->cart.mapper = map;
	}

	nes->cpu_bus.cartridge = &nes->cart;
	nes->cpu_bus.ppu = &nes->ppu;
	nes->cpu_bus.cpu = &nes->cpu;
	nes->cpu_bus.pad = &nes->pad;

	nes->ppu_bus.cartridge = &nes->cart;

	nes->cpu.bus = &nes->cpu_bus;

	nes->ppu.bus = &nes->ppu_bus;
	nes->ppu.cpu = &nes->cpu;

	NESReset(nes);
}

void NESDestroy(Nes* nes)
{
	free_cartridge(&nes->cart);
	memset(nes, 0, sizeof(Nes));
}

void NESReset(Nes* nes)
{
	nes->system_clock = 0;

	power_on_6502(&nes->cpu);
	reset_6502(&nes->cpu);

	power_on_2C02(&nes->ppu);
	reset_2C02(&nes->ppu);
}
