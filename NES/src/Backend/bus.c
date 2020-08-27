#include "bus.h"

void bus_write(Bus* b, uint16_t addr, uint8_t data)
{
	b->memory[addr] = data;
}

uint8_t bus_read(Bus* b, uint16_t addr)
{
	return b->memory[addr];
}