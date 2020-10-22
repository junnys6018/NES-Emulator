#ifndef NES_H
#define NES_H

#include "6502.h"
#include "6502_Bus.h"
#include "2C02.h"
#include "2C02_Bus.h"
#include "2A03.h"
#include "Cartridge.h"
#include "Gamepad.h"

typedef struct
{
	Bus6502 cpu_bus;
	Bus2C02 ppu_bus;
	State6502 cpu;
	State2C02 ppu;
	State2A03 apu;

	Cartridge cart;
	Gamepad pad;
	uint64_t system_clock;
} Nes;

void NESInit(Nes* nes, const char* filepath);
void NESDestroy(Nes* nes);
void NESReset(Nes* nes);

// Emulate one cpu instruction
inline void clock_nes_instruction(Nes* nes)
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

// Emulate a whole frame
inline void clock_nes_frame(Nes* nes)
{
	do
	{
		nes->system_clock++;
		if (nes->system_clock % 3 == 0)
			clock_6502(&nes->cpu);

		clock_2C02(&nes->ppu);
		clock_2A03(&nes->apu);
	} while (!(nes->ppu.scanline == 241 && nes->ppu.cycles == 0));
}

// Emulate one master clock cycle
inline void clock_nes_cycle(Nes* nes)
{
	nes->system_clock++;
	if (nes->system_clock % 3 == 0)
		clock_6502(&nes->cpu);

	clock_2C02(&nes->ppu);
	clock_2A03(&nes->apu);
}

#endif // !NES_H
