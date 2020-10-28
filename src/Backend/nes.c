#include "nes.h"
#include "Backend/Mappers/Mapper_JUN.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>

int NESInit(Nes* nes, const char* filepath)
{
	if (filepath)
	{
		if (load_cartridge_from_file(&nes->cart, filepath) != 0)
		{
			// Loading cart failed
			memset(nes, 0, sizeof(Nes));
			return 1;
		}

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
	nes->cpu_bus.apu = &nes->apu;
	nes->cpu_bus.pad = &nes->pad;

	nes->ppu_bus.cartridge = &nes->cart;

	nes->cpu.bus = &nes->cpu_bus;

	nes->ppu.bus = &nes->ppu_bus;
	nes->ppu.cpu = &nes->cpu;

	nes->apu.cpu = &nes->cpu;

	NESReset(nes);

	return 0;
}

void NESDestroy(Nes* nes)
{
	free_cartridge(&nes->cart);
	memset(nes, 0, sizeof(Nes));
}

void NESReset(Nes* nes)
{
	nes->system_clock = 0;
	nes->audio_time = 0.0f;

	power_on_6502(&nes->cpu);
	reset_6502(&nes->cpu);

	power_on_2C02(&nes->ppu);
	reset_2C02(&nes->ppu);

	power_on_2A03(&nes->apu);
	reset_2A03(&nes->apu);
}
