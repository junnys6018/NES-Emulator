#include "MapperJUN.h"
#include <stdlib.h>
#include <assert.h>

uint8_t mjun_cpu_read_cartridge(Cartridge* cart, uint16_t addr, bool* read)
{
	*read = true;
	MapperJUN* mapJUN = (MapperJUN*)cart->mapper;

	return mapJUN->PRG_RAM[addr];
}

void mjun_cpu_write_cartridge(Cartridge* cart, uint16_t addr, uint8_t data, bool* wrote)
{
	*wrote = true;
	MapperJUN* mapJUN = (MapperJUN*)cart->mapper;
	
	mapJUN->PRG_RAM[addr] = data;
}

uint8_t mjun_ppu_read_cartridge(Cartridge* cart, uint16_t addr)
{
	MapperJUN* mapJUN = (MapperJUN*)cart->mapper;
	return mapJUN->CHR[addr];
}

void mjun_ppu_write_cartridge(Cartridge* cart, uint16_t addr, uint8_t data)
{
	MapperJUN* mapJUN = (MapperJUN*)cart->mapper;
	mapJUN->CHR[addr] = data;
}

// Fixed horizontal mirroing
NametableIndex mjun_ppu_mirror_nametable(void* mapper, uint16_t addr)
{
	return mirror_horizontal(addr);
}

void mjun_load_from_file(Cartridge* cart, FILE* file)
{
	cart->mapper_id = 767; // Assign mapper_id 767 to my format

	cart->cpu_read_cartridge = mjun_cpu_read_cartridge;
	cart->cpu_write_cartridge = mjun_cpu_write_cartridge;

	cart->ppu_read_cartridge = mjun_ppu_read_cartridge;
	cart->ppu_peak_cartridge = mjun_ppu_read_cartridge;
	cart->ppu_write_cartridge = mjun_ppu_write_cartridge;

	cart->ppu_mirror_nametable = mjun_ppu_mirror_nametable;

	MapperJUN* map = malloc(sizeof(MapperJUN));
	assert(map);
	cart->mapper = map;

	fread(map->PRG_RAM, 64 * 1024, 1, file);
}
