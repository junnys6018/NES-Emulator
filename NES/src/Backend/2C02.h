#ifndef _2C02_H
#define _2C02_H
#include "2C02_Bus.h"

#include <stdbool.h>

// Forward declaration to avoid circular dependency
struct State6502;
void NMI(struct State6502* cpu);

typedef struct
{
	uint8_t r, g, b;
} color;
extern color PALETTE_MAP[64];

typedef struct
{
	// Registers exposed to CPU

	// Access: write only, accessed from $2000
	union
	{
		struct
		{
			uint8_t N : 2; // Base nametable address
			uint8_t I : 1; // VRAM increment
			uint8_t S : 1; // Sprite pattern table address for 8x8 sprites (0: $0000; 1: $1000)
			uint8_t B : 1; // Background pattern table address (0: $0000; 1: $1000)
			uint8_t H : 1; // Sprite size (0: 8x8 pixels; 1: 8x16 pixels)
			uint8_t P : 1; // PPU master/slave select (0: read backdrop from EXT pins; 1: output color on EXT pins)
			uint8_t V : 1; // Generate an NMI at the start of the vertical blanking interval (0: off; 1: on)
		} flags;
		uint8_t reg;
	} PPUCTRL;

	// Access: write only, accessed from $2001
	union
	{
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
		} flags;
		uint8_t reg;
	} PPUMASK;

	// Access: read only, accessed from $2002
	union
	{
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
			 * cleared after reading $2002 and at dot 1 of the pre-render line.
			 */
			uint8_t V : 1;
		} flags;
		uint8_t reg;
	} PPUSTATUS;

	// Access: write only, accessed from $2003
	uint8_t OAMADDR; // OAM address port

	// Access: read/write, accessed from $2004 
	uint8_t OAMDATA; 

	// Access: write twice, accessed from $2005
	uint8_t PPUSCROLL;

	// Access: write twice, accessed from $2006
	uint8_t PPUADDR;

	// Access: read/write, accessed from $2007
	uint8_t PPUDATA;

	// Internal Registers

	uint16_t v; // Current VRAM address (15 bits)
	uint16_t t; // Temporary VRAM address (15 bits)
	uint8_t x; // Fine X scroll (3 bits)
	bool w; // First or second write toggle
	bool oddframe; // 0: even; 1: odd

	// BG Rendering registers

	uint16_t pt_shift_low, pt_shift_high; // Pattern table shift registers
	uint16_t pa_shift_low, pa_shift_high; // Palatte attribute shift registers

	uint8_t pt_latch_low, pt_latch_high, pa_latch_low, pa_latch_high; // Latches used to feed data into shift registers
	uint8_t name_tbl_byte; // Byte fetched from name table
	
	// Sprite Rendering registers
	uint8_t active_sprites;
	uint8_t pt_sprite_low[8];
	uint8_t pt_sprite_high[8];
	uint8_t pa_sprite[8];
	uint8_t sprite_xpos[8];
	bool sprite_zero_on_next_scanline, sprite_zero_on_current_scanline;
	
	int cycles; // 341 cycles per scanline
	int scanline; // 262 scanlines per frame

	// Stuff that does not represent the hardware of the 2C02

	// Sprite evaluation runs like a state machine
	struct
	{
		// states represent 2.1...2.4 listed here: http://wiki.nesdev.com/w/index.php/PPU_sprite_evaluation
		enum 
		{
			RANGE_CHECK,
			OVERFLOW_CHECK, 
			IDLE, 		
		} state;
		bool write_enable;
		uint8_t secondary_oam_free_slot;
		uint8_t remaining;
	} sprite_eval_state;

	uint64_t total_cycles;

	color pixels[256 * 240];
	Bus2C02* bus;
	struct State6502* cpu;
	
	// Due to hardware, strange behaviour occurs if PPUSTATUS is read at the beggining of vertical blanking 
	// If PPUSTATUS is read at (0,241), the V flag is read as clear, and it wont be set at (1,241), and NMI will not be generated
	// If PPUSTATUS is read at (1,241) or (2,241), the V flag is read as set, and will be cleared as usual, but NMI will be supressed
	bool ppustatus_read_early, ppustatus_read_late;

	uint32_t nmi_line; // bitfield of 32 past signal levels of the nmi line. bit 0 is the most recent signal level

} State2C02;

void clock_2C02(State2C02* ppu);
void reset_2C02(State2C02* ppu);
void power_on_2C02(State2C02* ppu);
void write_ppu(State2C02* ppu, uint16_t addr, uint8_t data);
uint8_t read_ppu(State2C02* ppu, uint16_t addr);
#endif // !_2C02_H
