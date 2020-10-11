#ifndef MAPPER_UTILS_H
#define MAPPER_UTILS_H
#include "Backend/Cartridge.h"

typedef enum
{
	ONE_SCREEN_LOWER, ONE_SCREEN_UPPER, VERTICAL, HORIZONTAL
} MirrorMode;

NametableIndex MirrorVertical(uint16_t addr);
NametableIndex MirrorHorizontal(uint16_t addr);
NametableIndex MirrorOneScreenLower(uint16_t addr);
NametableIndex MirrorOneScreenUpper(uint16_t addr);

#endif // !MAPPER_UTILS_H
