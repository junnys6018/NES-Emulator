#include "nes.h"
#include "Mappers/MapperJUN.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>

int initialize_nes(Nes* nes, const char* filepath, UPDATE_PATTERN_TABLE_CB callback)
{
	memset(nes, 0, sizeof(Nes));
	if (filepath)
	{
		if (load_cartridge_from_file(nes, filepath, callback) != 0)
		{
			// Loading cart failed
			return 1;
		}
	}
	else // Load a dummy cart
	{
		memset(&nes->cart, 0, sizeof(nes->cart));
		nes->cart.mapper_id = 767;

		nes->cart.cpu_read_cartridge = mjun_cpu_read_cartridge;
		nes->cart.cpu_write_cartridge = mjun_cpu_write_cartridge;

		nes->cart.ppu_read_cartridge = mjun_ppu_read_cartridge;
		nes->cart.ppu_peak_cartridge = mjun_ppu_read_cartridge;
		nes->cart.ppu_write_cartridge = mjun_ppu_write_cartridge;

		nes->cart.ppu_mirror_nametable = mjun_ppu_mirror_nametable;

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

	reset_nes(nes);

	return 0;
}

void destroy_nes(Nes* nes)
{
	free_cartridge(&nes->cart);
	memset(nes, 0, sizeof(Nes));
}

void reset_nes(Nes* nes)
{
	nes->system_clock = 0;

	power_on_6502(&nes->cpu);
	reset_6502(&nes->cpu);

	power_on_2C02(&nes->ppu);
	reset_2C02(&nes->ppu);

	power_on_2A03(&nes->apu);
	reset_2A03(&nes->apu);
}

void clock_nes_instruction(Nes* nes)
{
	while (true)
	{
		nes->system_clock++;
		if (nes->system_clock % 3 == 0 && clock_6502(&nes->cpu) == 0)
		{
			clock_2C02(&nes->ppu);
			break;
		}
		clock_2C02(&nes->ppu);
		clock_2A03(&nes->apu);
	}
}

void clock_nes_frame(Nes* nes)
{
	do
	{
		clock_nes_cycle(nes);
	} while (!(nes->ppu.scanline == 242 && nes->ppu.cycles == 0));
}