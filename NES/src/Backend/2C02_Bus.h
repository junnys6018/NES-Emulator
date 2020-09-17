#ifndef _2C02_BUS_H
#define _2C02_BUS_H
#include <stdint.h>

#include "Cartridge.h"

typedef struct
{
	Cartridge* cartridge;
	
	uint8_t OAM[256]; // Object Attribute Memory
	uint8_t nametable[2][1024];
	uint8_t palette[32]; // 32 Bytes of palette data
} Bus2C02;

uint8_t ppu_bus_read(Bus2C02* bus, uint16_t addr);
void ppu_bus_write(Bus2C02* bus, uint16_t addr, uint8_t data);
#endif // !_2C02_BUS_H
