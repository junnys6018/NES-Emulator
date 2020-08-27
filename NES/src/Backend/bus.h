#ifndef BUS_H
#define BUS_H

#include <stdint.h>

typedef struct
{
	uint8_t memory[64 * 1024]; // 64 KB of memory
} Bus;

void bus_write(Bus* b, uint16_t addr, uint8_t data);
uint8_t bus_read(Bus* b, uint16_t addr);

#endif
