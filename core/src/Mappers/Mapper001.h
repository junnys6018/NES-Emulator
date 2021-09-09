#ifndef MAPPER_001_H
#define MAPPER_001_H
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "../Cartridge.h"

typedef struct
{
	// Mapper Registers (All 5 bits wide, use lower 5 bits and ignore 3 upper bits)
	uint8_t shift_register; // 5 bits

	union
	{
		struct
		{
			uint8_t M : 2; // Mirror mode (0: one-screen lower; 1: one-screen upper; 2: vertical; 3: horizontal)
			uint8_t P : 2; // PRG_ROM bank mode (0,1: switch 32KB at $8000; 2: fix first bank at $8000 and switch 16 KB bank at $C000; 3: fix last bank at $C000 and switch 16 KB bank at $8000))
			uint8_t C : 1; // CHR_ROM bank mode (0: swtich 8KB at a time; 1: swtich 2x4KB banks independently)
			uint8_t Unused : 3;
		} bits;
		uint8_t reg;
	} control; // 5 bits

	uint8_t CHR_bank0_select; // 5 bits
	uint8_t CHR_bank1_select; // 5 bits
	uint8_t PRG_bank_select;  // 5 bits

	// Banks

	// Mapped from $6000-$7FFF
	uint8_t* PRG_RAM;

	// Mapped from $8000-$FFFF
	uint8_t* PRG_ROM;

	uint8_t* CHR;

	uint16_t PRG_ROM_banks;
	uint16_t PRG_RAM_banks;
	uint16_t CHR_banks;
} Mapper001;

void m001_free(Mapper001* mapper);
void m001_load_from_file(Header* header, Cartridge* cart, FILE* file);

#endif
