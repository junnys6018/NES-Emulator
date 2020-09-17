#ifndef _2C02_H
#define _2C02_H
#include "2C02_Bus.h"

typedef struct
{
	// Access: write only, accessed from $2000
	struct
	{
		uint8_t N : 2; // Base nametable address
		uint8_t I : 1; // VRAM increment
		uint8_t S : 1; // Sprite pattern table address for 8x8 sprites (0: $0000; 1: $1000)
		uint8_t B : 1; // Background pattern table address (0: $0000; 1: $1000)
		uint8_t H : 1; // Sprite size (0: 8x8 pixels; 1: 8x16 pixels)
		uint8_t P : 1; // PPU master/slave select (0: read backdrop from EXT pins; 1: output color on EXT pins)
		uint8_t V : 1; // Generate an NMI at the start of the vertical blanking interval (0: off; 1: on)
	} PPUCTRL;

	// Access: write only, accessed from $2001
	struct
	{
		uint8_t g : 1; // Greyscale (0: normal color, 1: produce a greyscale display)
		uint8_t m : 1; // Show background in leftmost 8 pixels of screen, 0: Hide
		uint8_t M : 1; // Show sprites in leftmost 8 pixels of screen, 0: Hide
		uint8_t b : 1; // Show background
		uint8_t s : 1; // Show sprites
		uint8_t R : 1; // Emphasize red
		uint8_t G : 1; // Emphasize green
		uint8_t B : 1; // Emphasize blue
	} PPUMASK;

	// Access: read only, accessed from $2002
	struct
	{
		uint8_t LSB : 5; // 5 Least significant bits last witten to a PPU register

		/* * * * * * * * * 
		 * Sprite overflow. The intent was for this flag to be set
         * whenever more than eight sprites appear on a scanline, but a
         * hardware bug causes the actual behavior to be more complicated
         * and generate false positives as well as false negatives; see
         * http://wiki.nesdev.com/w/index.php/PPU_sprite_evaluation. This 
		 * flag is set during sprite evaluation and cleared at dot 1 
         * (the second dot) of the pre-render line.
		 * * * * * * * * */
		uint8_t O : 1;

		/* * * * * * * * * 
		 * Sprite 0 Hit. Set when a nonzero pixel of sprite 0 overlaps
         * a nonzero background pixel; cleared at dot 1 of the pre-render
         * line. Used for raster timing.
		 * * * * * * * * */
		uint8_t S : 1;

		/*
		 * Vertical blank has started (0: not in vblank; 1: in vblank).
		 * Set at dot 1 of line 241 (the line *after* the post-render line);
		 * cleared after reading $2002and at dot 1 of the pre-render line.
		 */
		uint8_t V : 1;
	} PPUSTATUS;

	// Access: write only, accessed from $2003
	uint8_t OAMADDR; // OAM address port

	// Access: read/write, accessed from $2004 
	uint8_t OAMDATA; 

	// Access: write twice, accessed from $2005
	uint8_t PPUSCROLL;

	// Access: write twice, accessed from $2006
	uint8_t PPUADDR;

	// Access: read/write, accessed from $2006
	uint8_t PPUDATA;

	// Access: write, accessed from $4014
	uint8_t OAMDMA;

	Bus2C02* bus;

} State2C02;

void clock_2C02(State2C02* ppu);
void write_ppu(State2C02* ppu, uint16_t addr);
uint8_t read_ppu(State2C02* ppu, uint16_t addr);
#endif // !_2C02_H
