#ifndef _6502_BUS_H
#define _6502_BUS_H
#include <stdint.h>

#include "2A03.h"
#include "2C02.h"
#include "Cartridge.h"
#include "Cheatcodes/cheatcode.h"
#include "Gamepad.h"

typedef struct
{
	uint8_t memory[2 * 1024]; // 2KB of memory
	Cartridge* cartridge;
	State2C02* ppu;
	struct State6502* cpu;
	State2A03* apu;
	Gamepad* pad;

	CheatCodeSystem cheats;
} Bus6502;

uint8_t cpu_bus_read(Bus6502* b, uint16_t addr);
void cpu_bus_write(Bus6502* b, uint16_t addr, uint8_t data);

#endif
