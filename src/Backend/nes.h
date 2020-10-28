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
	float audio_time;
} Nes;

// Passing NULL as filepath loads the nes into a "dummy state"
// Each successful call to NESInit() must be paired with a NESDestroy() to free resources
// If NESInit() fails (returns non zero) then NESDestroy() should NOT be called
// Returns 0 on success; non zero on failure
int NESInit(Nes* nes, const char* filepath);
void NESDestroy(Nes* nes);

// Reset button on the NES
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

// Emulate one master clock cycle, returns true when an audio sample is ready
// This is for syncing to audio

#define SAMPLE_RATE  (41000)
#define SAMPLE_PERIOD (1.0f / SAMPLE_RATE)
inline bool clock_nes_cycle(Nes* nes)
{
	nes->system_clock++;
	if (nes->system_clock % 3 == 0)
		clock_6502(&nes->cpu);

	clock_2C02(&nes->ppu);
	clock_2A03(&nes->apu);

	nes->audio_time += 1.0f / 5369318.0f; // PPU Clock Frequency
	if (nes->audio_time > SAMPLE_PERIOD)
	{
		nes->audio_time -= SAMPLE_PERIOD;
		return true;
	}
	else
	{
		return false;
	}
}

#endif // !NES_H
