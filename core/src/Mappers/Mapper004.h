#ifndef MAPPER_004_H
#define MAPPER_004_H
#include "6502.h"
#include "Cartridge.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

typedef struct
{
	// Mapped from $6000-$7FFF
	uint8_t PRG_RAM[8 * 1024];

	// Mapped from $8000-$FFFF
	uint8_t* PRG_ROM;

	uint8_t* CHR_ROM;

	// Memory Mapping Registers

	// ($8000-$9FFE, even)
	union
	{
		struct
		{
			uint8_t R : 3; // Specify which bank register to update on next write to Bank Data register
			uint8_t Unused : 3;
			uint8_t P : 1; // PRG ROM bank mode (0: $8000-$9FFF swappable,
						   //                       $C000 - $DFFF fixed to second - last bank;
						   //                    1: $C000-$DFFF swappable,
						   //                       $8000-$9FFF fixed to second-last bank)

			uint8_t C : 1; // CHR A12 inversion (0: two 2 KB banks at $0000-$0FFF,
						   //                       four 1 KB banks at $1000 - $1FFF;
						   //                       1: two 2 KB banks at $1000-$1FFF,
						   //                       four 1 KB banks at $0000-$0FFF)
		} bits;
		uint8_t reg;
	} bank_select;

	// ($8001-$9FFF, odd)
	// bank_data;

	// ($A000-$BFFE, even)
	bool mirroring; // (0: vertical; 1: horizontal)

	// ($A001-$BFFF, odd)
	union
	{
		struct MyStruct
		{
			uint8_t Unused : 6;
			uint8_t W : 1; // Write protection (0: allow writes; 1: deny writes)
			uint8_t R : 1; // PRG RAM chip enable (0: disable; 1: enable)
		} bits;
		uint8_t reg;
	} ram_protect;

	// IRQ registers

	// ($C000-$DFFE, even)
	uint8_t IRQ_latch;

	// ($C001-$DFFF, odd)
	bool IRQ_reload;

	bool IRQ_enable; // write to ($E000-$FFFE, even) to disable. ($E001-$FFFF, odd) to enable

	// Internal data
	uint8_t IRQ_counter;
	bool old_PPU_A12;

	uint32_t PRG_ROM_banks;
	uint32_t CHR_banks;

	uint8_t PRG_bank0_select;
	uint8_t PRG_bank1_select;

	uint8_t CHR_bank0_select;
	uint8_t CHR_bank1_select;
	uint8_t CHR_bank2_select;
	uint8_t CHR_bank3_select;
	uint8_t CHR_bank4_select;
	uint8_t CHR_bank5_select;

	State6502* cpu;

	uint8_t left_pt[4096];
	uint8_t right_pt[4096];
} Mapper004;

void m004_free(Mapper004* mapper);
void m004_load_from_file(Header* header, Cartridge* cart, FILE* file, State6502* cpu);
int m004_save_game(Cartridge* cart, FILE* savefile, char error_string[256]);
int m004_load_save(Cartridge* cart, FILE* savefile, char error_string[256]);

#endif
