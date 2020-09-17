#ifndef _6502_BUS_H
#define _6502_BUS_H
#include <stdint.h>

#include "Cartridge.h"
#include "2C02.h"
#define BUS_TEST 0
#if BUS_TEST
typedef struct
{
	uint8_t memory[64 * 1024]; // 64 KB of memory
} Bus6502;
#else
typedef struct
{
	uint8_t memory[2 * 1024]; // 2KB of memory
	Cartridge* cartridge;
	State2C02* ppu;
} Bus6502;

#endif
void cpu_bus_write(Bus6502* b, uint16_t addr, uint8_t data);
uint8_t cpu_bus_read(Bus6502* b, uint16_t addr);

#endif
