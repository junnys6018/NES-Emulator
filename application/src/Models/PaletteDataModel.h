#ifndef PALETTE_DATA_MODEL_H
#define PALETTE_DATA_MODEL_H
#include <stdint.h>

typedef struct
{
	// Pointer to the PPU's palette data
	uint8_t* pal;
} PaletteDataModel;

#endif