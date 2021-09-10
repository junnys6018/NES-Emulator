#ifndef MAPPER_JUN_H
#define MAPPER_JUN_H
#include <stdint.h>
#include <stdio.h>
#include "Cartridge.h"
#include "MapperUtils.h"

typedef struct
{
	uint8_t PRG_RAM[64 * 1024];
	uint8_t CHR[8 * 1024];
} MapperJUN;

uint8_t mjun_cpu_read_cartridge(Cartridge* cart, uint16_t addr, bool* read);
void mjun_cpu_write_cartridge(Cartridge* cart, uint16_t addr, uint8_t data, bool* wrote);

uint8_t mjun_ppu_read_cartridge(Cartridge* cart, uint16_t addr);
void mjun_ppu_write_cartridge(Cartridge* cart, uint16_t addr, uint8_t data);

NametableIndex mjun_ppu_mirror_nametable(void* mapper, uint16_t addr);

void mjun_load_from_file(Cartridge* cart, FILE* file);

#endif