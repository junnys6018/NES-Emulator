#ifndef NES_H
#define NES_H

#include "6502.h"
#include "6502Bus.h"
#include "2C02.h"
#include "2C02Bus.h"
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

// Each successful call to NESInit() must be paired with a destroy_nes() to free resources
// If NESInit() fails (returns non zero) then destroy_nes() should NOT be called
// Returns 0 on success; non zero on failure
int initialize_nes(Nes* nes, const char* filepath, UPDATE_PATTERN_TABLE_CB callback, char error_string[256]);
void destroy_nes(Nes* nes);

// Reset button on the NES
void reset_nes(Nes* nes);

// Emulate one cpu instruction
void clock_nes_instruction(Nes* nes);

// Emulate a whole frame
void clock_nes_frame(Nes* nes);

// Emulate one PPU cycle
inline void clock_nes_cycle(Nes* nes)
{
	nes->system_clock++;
	if (nes->system_clock % 3 == 0)
		clock_6502(&nes->cpu);

	clock_2C02(&nes->ppu);
	clock_2A03(&nes->apu);
}


#endif // !NES_H
