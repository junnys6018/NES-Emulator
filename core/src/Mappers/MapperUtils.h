#ifndef MAPPER_UTILS_H
#define MAPPER_UTILS_H
#include "Cartridge.h"

typedef enum
{
	ONE_SCREEN_LOWER,
	ONE_SCREEN_UPPER,
	VERTICAL,
	HORIZONTAL,
	FOUR_SCREEN
} MirrorMode;

NametableIndex mirror_vertical(uint16_t addr);
NametableIndex mirror_horizontal(uint16_t addr);
NametableIndex mirror_one_screen_lower(uint16_t addr);
NametableIndex mirror_one_screen_upper(uint16_t addr);

#endif
