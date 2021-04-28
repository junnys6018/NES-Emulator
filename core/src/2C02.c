#include "2C02.h"

#include <stdio.h> // For printf
#include <string.h> // For memset
#include <assert.h>

// TODO: Implement color emphasis
// TODO: Implement "show bg and sprites in leftmost 8 pixels of screen flag"

// Maps a 6 bit HSV color into RGB
color PALETTE_MAP[64] =
{
	{ 84,  84,  84}, {  0,  30, 116}, {  8,  16, 144}, { 48,   0, 136}, { 68,   0, 100}, { 92,   0,  48}, { 84,   4,   0}, { 60,  24,   0}, { 32,  42,   0}, {  8,  58,   0}, {  0,  64,   0}, {  0,  60,   0}, {  0,  50,  60}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
	{152, 150, 152}, {  8,  76, 196}, { 48,  50, 236}, { 92,  30, 228}, {136,  20, 176}, {160,  20, 100}, {152,  34,  32}, {120,  60,   0}, { 84,  90,   0}, { 40, 114,   0}, {  8, 124,   0}, {  0, 118,  40}, {  0, 102, 120}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
	{236, 238, 236}, { 76, 154, 236}, {120, 124, 236}, {176,  98, 236}, {228,  84, 236}, {236,  88, 180}, {236, 106, 100}, {212, 136,  32}, {160, 170,   0}, {116, 196,   0}, { 76, 208,  32}, { 56, 204, 108}, { 56, 180, 204}, { 60,  60,  60}, {  0,   0,   0}, {  0,   0,   0},
	{236, 238, 236}, {168, 204, 236}, {188, 188, 236}, {212, 178, 236}, {236, 174, 236}, {236, 174, 212}, {236, 180, 176}, {228, 196, 144}, {204, 210, 120}, {180, 222, 120}, {168, 226, 144}, {152, 226, 180}, {160, 214, 228}, {160, 162, 160}, {  0,   0,   0}, {  0,   0,   0}
};

uint16_t CoarseXInc(uint16_t v)
{
	if ((v & 0x001F) == 31) // if coarse X == 31
	{
		v &= ~0x001F; // coarse X = 0
		v ^= 0x0400; // switch horizontal nametable
	}
	else
	{
		v += 1; // increment coarse X
	}

	return v;
}

uint16_t FineYInc(uint16_t v)
{
	if ((v & 0x7000) != 0x7000) // if fine Y < 7
	{
		v += 0x1000; // increment fine Y
	}
	else
	{
		v &= ~0x7000; // fine Y = 0
		int y = (v & 0x03E0) >> 5; // get coarse y
		if (y == 29)
		{
			y = 0; // coarse Y = 0
			v ^= 0x0800; // switch vertical nametable
		}
		else if (y == 31)
		{
			y = 0; // coarse Y = 0, nametable not switched
		}
		else
		{
			y += 1; // increment coarse Y
		}

		v = (v & ~0x03E0) | (y << 5); // put coarse Y back into v
	}

	return v;
}

uint16_t GetNameTableAddr(uint16_t v)
{
	return 0x2000 | (v & 0x0FFF);
}

uint16_t GetAttribTableAddr(uint16_t v)
{
	return 0x23C0 | (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07);
}

void FetchDataAndIncV(State2C02* ppu)
{
	switch (ppu->cycles % 8)
	{
	case 1:
	{
		// NT byte
		uint16_t name_tbl_addr = GetNameTableAddr(ppu->v);
		ppu->name_tbl_byte = ppu_bus_read(ppu->bus, name_tbl_addr);
		break;
	}
	case 3:
	{
		// AT byte
		uint16_t attrib_tbl_addr = GetAttribTableAddr(ppu->v);
		uint8_t attrib_tbl_byte = ppu_bus_read(ppu->bus, attrib_tbl_addr); // Get palette attribute
		uint8_t shift = 2 * ((ppu->v >> 1) & 0x01) + 4 * ((ppu->v >> 6) & 0x01); // Calculate which palette to use
		uint8_t palatteID = (attrib_tbl_byte >> shift) & 0x03;

		ppu->pa_latch_low = ((palatteID & 0x01) ? 0xFF : 0x00);
		ppu->pa_latch_high = ((palatteID & 0x02) ? 0xFF : 0x00);
		break;
	}
	case 5:
	{
		// Low PT byte
		int fine_y = (ppu->v >> 12) & 0x7;
		uint16_t pattern_tbl_addr = ppu->PPUCTRL.flags.B << 12 | ppu->name_tbl_byte << 4 | fine_y;
		ppu->pt_latch_low = ppu_bus_read(ppu->bus, pattern_tbl_addr);
		break;
	}
	case 7:
	{
		// High PT byte
		int fine_y = (ppu->v >> 12) & 0x7;
		uint16_t pattern_tbl_addr = ppu->PPUCTRL.flags.B << 12 | ppu->name_tbl_byte << 4 | (1 << 3) | fine_y;
		ppu->pt_latch_high = ppu_bus_read(ppu->bus, pattern_tbl_addr);
		break;
	}
	case 0:
	{
		if (ppu->cycles == 256)
		{
			ppu->v = FineYInc(ppu->v);
		}
		else
		{
			ppu->v = CoarseXInc(ppu->v);
		}
		break;
	}
	}
}

// Load new bytes into shift registers
void FeedShiftRegisters(State2C02* ppu)
{
	ppu->pt_shift_low = (ppu->pt_shift_low & 0xFF00) | (uint16_t)ppu->pt_latch_low;
	ppu->pt_shift_high = (ppu->pt_shift_high & 0xFF00) | (uint16_t)ppu->pt_latch_high;
	ppu->pa_shift_low = (ppu->pa_shift_low & 0xFF00) | (uint16_t)ppu->pa_latch_low;
	ppu->pa_shift_high = (ppu->pa_shift_high & 0xFF00) | (uint16_t)ppu->pa_latch_high;
}

void Shift(State2C02* ppu)
{
	ppu->pt_shift_low <<= 1;
	ppu->pt_shift_high <<= 1;
	ppu->pa_shift_low <<= 1;
	ppu->pa_shift_high <<= 1;
}

void TransitionSpriteEvalulationStateMachine(State2C02* ppu)
{
	if (ppu->OAMADDR == 0) // Overflow
	{
		ppu->sprite_eval_state.state = IDLE;
	}
	else if (ppu->sprite_eval_state.secondary_oam_free_slot < 32) // Less than 8 sprites found
	{
		ppu->sprite_eval_state.state = RANGE_CHECK;
	}
	else
	{
		ppu->sprite_eval_state.state = OVERFLOW_CHECK;
	}
}

void SwapFrameBuffers(State2C02* ppu)
{
	ppu->back_buffer = 1 - ppu->back_buffer;
#ifdef __EMSCRIPTEN__
	memset(ppu->pixels[ppu->back_buffer], 0, 256 * 240 * 4);
#else
	memset(ppu->pixels[ppu->back_buffer], 0, 256 * 240 * 3);
#endif
}

void clock_2C02(State2C02* ppu)
{
	uint8_t bg_shade = 0;
	// Background rendering
	if (ppu->PPUMASK.flags.b)
	{
		// Pre render line
		if (ppu->scanline == -1)
		{
			if (ppu->cycles % 8 == 1 && ((ppu->cycles >= 9 && ppu->cycles <= 257) || (ppu->cycles >= 321 && ppu->cycles <= 337)))
			{
				FeedShiftRegisters(ppu);
			}

			if (ppu->cycles >= 1 && ppu->cycles < 257)
			{
				Shift(ppu);
				FetchDataAndIncV(ppu);
			}
			else if (ppu->cycles == 257)
			{
				// Copy horizontal data from t to v
				ppu->v = (ppu->v & 0x7BE0) | (ppu->t & ~0x7BE0);
			}
			else if (ppu->cycles >= 280 && ppu->cycles < 305)
			{
				// Copy vertical data from t to v
				ppu->v = (ppu->v & 0x041F) | (ppu->t & ~0x041F);
			}
			else if (ppu->cycles >= 321 && ppu->cycles < 337)
			{
				Shift(ppu);
				FetchDataAndIncV(ppu);
			}
		}
		// Visible Scanlines
		else if (ppu->scanline >= 0 && ppu->scanline < 240)
		{
			if (ppu->cycles % 8 == 1 && ((ppu->cycles >= 9 && ppu->cycles <= 257) || (ppu->cycles >= 321 && ppu->cycles <= 337)))
			{
				FeedShiftRegisters(ppu);
			}

			if (ppu->cycles >= 1 && ppu->cycles < 257)
			{
				uint16_t bit_mask = 0x8000 >> ppu->x;

				uint8_t pixel_low = (ppu->pt_shift_low & bit_mask) > 0;
				uint8_t pixel_high = (ppu->pt_shift_high & bit_mask) > 0;
				bg_shade = (pixel_high << 1) | pixel_low;

				uint8_t pal_low = (ppu->pa_shift_low & bit_mask) > 0;
				uint8_t pal_high = (ppu->pa_shift_high & bit_mask) > 0;
				uint8_t palatte = (pal_high << 1) | pal_low;

				uint8_t color; // color to be drawn to the screen during the visible scanlines
				if (bg_shade == 0) // Universal background color
				{
					color = ppu_bus_read(ppu->bus, 0x3F00) & 0x3F;
				}
				else
				{
					uint16_t palatte_addr = 0x3F00 | palatte << 2 | bg_shade;
					color = ppu_bus_read(ppu->bus, palatte_addr) & 0x3F;
				}
#ifdef __EMSCRIPTEN__
				int index = 4 * (ppu->scanline * 256 + ppu->cycles - 1);
#else
				int index = 3 * (ppu->scanline * 256 + ppu->cycles - 1);
#endif
				if (ppu->PPUMASK.flags.g)
				{
					color &= 0x30;
				}
				ppu->pixels[ppu->back_buffer][index + 0] = PALETTE_MAP[color].r;
				ppu->pixels[ppu->back_buffer][index + 1] = PALETTE_MAP[color].g;
				ppu->pixels[ppu->back_buffer][index + 2] = PALETTE_MAP[color].b;
#ifdef __EMSCRIPTEN__
				ppu->pixels[ppu->back_buffer][index + 3] = 255;
#endif
				Shift(ppu);
				FetchDataAndIncV(ppu);
			}
			else if (ppu->cycles == 257)
			{
				// Copy horizontal data from t to v
				ppu->v = (ppu->v & 0x7BE0) | (ppu->t & ~0x7BE0);
			}
			else if (ppu->cycles >= 321 && ppu->cycles < 337)
			{
				Shift(ppu);
				FetchDataAndIncV(ppu);
			}
		}
	}

	// Sprite rendering
	if (ppu->PPUMASK.flags.s)
	{
		if (ppu->scanline >= 1 && ppu->scanline < 240)
		{
			if (ppu->cycles >= 1 && ppu->cycles < 257)
			{
				// For each sprite on the current scanline
				for (int i = 0; i < 8; i++)
				{
					// Check if any sprite become active
					if (ppu->sprite_xpos[i] == 0)
					{
						ppu->active_sprites |= (1 << i);
					}
					ppu->sprite_xpos[i]--;
				}

				bool drawn = false;

				// For each active sprite
				for (int i = 0; i < 8; i++)
				{
					if (ppu->active_sprites & (1 << i))
					{
						uint8_t spr_shade;
						// Get the shade of the active sprite
						if (ppu->sprite_attribute[i] & (1 << 6)) // Flip horizontally
						{
							spr_shade = (ppu->pt_sprite_high[i] & 0x1) << 1 | (ppu->pt_sprite_low[i] & 0x1);
							ppu->pt_sprite_high[i] >>= 1;
							ppu->pt_sprite_low[i] >>= 1;
						}
						else
						{
							spr_shade = ((ppu->pt_sprite_high[i] & 0x80) >> 6) | ((ppu->pt_sprite_low[i] & 0x80) >> 7);
							ppu->pt_sprite_high[i] <<= 1;
							ppu->pt_sprite_low[i] <<= 1;
						}

						// If the sprite shade is not transparent and we havent rendered a sprite yet
						if (spr_shade != 0 && !drawn)
						{
							drawn = true;
							// Check for sprite 0 hit
							if (i == 0 && ppu->sprite_zero_on_current_scanline && bg_shade != 0 && ppu->PPUMASK.flags.b && !ppu->PPUSTATUS.flags.S)
							{
								ppu->PPUSTATUS.flags.S = 1;
								// Disable if left clipping is enabled and at the rightmost column 
								if ((!ppu->PPUMASK.flags.M || !ppu->PPUMASK.flags.m) && ppu->cycles >= 1 && ppu->cycles <= 8)
								{
									ppu->PPUSTATUS.flags.S = 0;
								}
								if (ppu->cycles == 256)
								{
									ppu->PPUSTATUS.flags.S = 0;
								}
							}

							// Sprite pixel only gets rasterized if the background is transparent
							// or priority bit is set in the sprites attribute byte
							if (bg_shade == 0 || !(ppu->sprite_attribute[i] & 0x20)) // Get priortiy bit from sprite attribute
							{
								uint16_t palatte_addr = 0x3F10 | (ppu->sprite_attribute[i] & 0x3) << 2 | spr_shade;
								uint8_t color = ppu_bus_read(ppu->bus, palatte_addr) & 0x3F;
								if (ppu->PPUMASK.flags.g)
								{
									color &= 0x30;
								}
#ifdef __EMSCRIPTEN__
								int index = 4 * (ppu->scanline * 256 + ppu->cycles - 1);
#else
								int index = 3 * (ppu->scanline * 256 + ppu->cycles - 1);
#endif
								ppu->pixels[ppu->back_buffer][index + 0] = PALETTE_MAP[color].r;
								ppu->pixels[ppu->back_buffer][index + 1] = PALETTE_MAP[color].g;
								ppu->pixels[ppu->back_buffer][index + 2] = PALETTE_MAP[color].b;
#ifdef __EMSCRIPTEN__
								ppu->pixels[ppu->back_buffer][index + 3] = 255;
#endif
							}
						}
					}
				}
			}
			// Clear active sprites at the end of the visible scanline
			else if (ppu->cycles == 257)
			{
				ppu->active_sprites = 0;
			}

		}
	}

	// Sprite evaluation, only occurs if rendering is enabled
	if (ppu->PPUMASK.flags.b || ppu->PPUMASK.flags.s)
	{
		// Sprite evaluation occurs during the visible scanlines (0..239). 
		// Sprites evaluated on the current scanline are drawn on the next scanline
		// because of this no sprites will be rendered on scanline 0 (the first scanline)
		if (ppu->scanline >= 0 && ppu->scanline < 240)
		{
			// Clear OAM
			if (ppu->cycles == 1) // during cycles 1..64, secondary OAM is cleared to 0xFF, we clear all of it at cycle 1
			{
				memset(ppu->bus->secondary_OAM, 0xFF, 32);

				// Set up initial state
				ppu->sprite_eval_state.state = RANGE_CHECK;
				ppu->sprite_eval_state.secondary_oam_free_slot = 0;
				ppu->sprite_eval_state.remaining = 0;

				ppu->sprite_zero_on_next_scanline = false;
			}
			// Sprite Evaluation state machine is run from cycles 65..256
			else if (ppu->cycles >= 65 && ppu->cycles < 257)
			{
				if (ppu->sprite_eval_state.remaining == 0)
				{
					switch (ppu->sprite_eval_state.state)
					{
					case RANGE_CHECK:
					{
						// Read y position of the next sprite
						ppu->OAMDATA = ppu->bus->OAM[ppu->OAMADDR];
						ppu->OAMADDR++;
						ppu->bus->secondary_OAM[ppu->sprite_eval_state.secondary_oam_free_slot] = ppu->OAMDATA;
						// Perform y-range check
						int height = ppu->PPUCTRL.flags.H ? 16 : 8;
						if (ppu->scanline >= ppu->OAMDATA && ppu->scanline < ppu->OAMDATA + height)
						{
							// If sprite zero passed the range check
							if (ppu->OAMADDR == 1)
							{
								ppu->sprite_zero_on_next_scanline = true;
							}
							ppu->sprite_eval_state.secondary_oam_free_slot++;

							// Copy tile index byte into secondary OAM
							ppu->OAMDATA = ppu->bus->OAM[ppu->OAMADDR];
							ppu->OAMADDR++;
							ppu->bus->secondary_OAM[ppu->sprite_eval_state.secondary_oam_free_slot] = ppu->OAMDATA;
							ppu->sprite_eval_state.secondary_oam_free_slot++;

							// Copy attribute byte into secondary OAM
							ppu->OAMDATA = ppu->bus->OAM[ppu->OAMADDR];
							ppu->OAMADDR++;
							ppu->bus->secondary_OAM[ppu->sprite_eval_state.secondary_oam_free_slot] = ppu->OAMDATA;
							ppu->sprite_eval_state.secondary_oam_free_slot++;

							// Copy x position byte into secondary OAM
							ppu->OAMDATA = ppu->bus->OAM[ppu->OAMADDR];
							ppu->OAMADDR++;
							ppu->bus->secondary_OAM[ppu->sprite_eval_state.secondary_oam_free_slot] = ppu->OAMDATA;
							ppu->sprite_eval_state.secondary_oam_free_slot++;

							// Do nothing for 7 cycles
							ppu->sprite_eval_state.remaining = 8;
						}
						else // Sprite failed range check
						{
							// Do nothing for 1 cycle
							ppu->sprite_eval_state.remaining = 2;

							// goto next sprite
							ppu->OAMADDR += 3;
						}
						TransitionSpriteEvalulationStateMachine(ppu);
						break;
					}
					case OVERFLOW_CHECK:
					{
						// Read y position of the next sprite
						ppu->OAMDATA = ppu->bus->OAM[ppu->OAMADDR];

						// Perform y-range check
						int height = ppu->PPUCTRL.flags.H ? 16 : 8;

						// Check for sprite overflow
						if (ppu->scanline >= ppu->OAMDATA && ppu->scanline < ppu->OAMDATA + height)
						{
							ppu->PPUSTATUS.flags.O = 1;
							ppu->OAMADDR += 4;
							ppu->sprite_eval_state.remaining = 8; // TODO: Confirm this timing
							ppu->sprite_eval_state.state = IDLE;
						}
						else
						{
							// Emulate sprite overflow bug 
							uint8_t oam_addr_low = ppu->OAMADDR & 0x03;
							uint8_t oam_addr_high = ppu->OAMADDR >> 2;

							oam_addr_low++;
							oam_addr_low &= 0x03;

							oam_addr_high++;
							oam_addr_high &= 0x3F;

							ppu->OAMADDR = (oam_addr_high << 2) | oam_addr_low;

							if (oam_addr_high == 0) // if we have checked all sprites for overflow
							{
								ppu->sprite_eval_state.state = IDLE;
							}
							else // continue checking for overflow
							{
								ppu->sprite_eval_state.state = OVERFLOW_CHECK;
							}

							ppu->sprite_eval_state.remaining = 2;
						}
						break;
					}
					case IDLE:
						ppu->OAMADDR += 4;
						ppu->sprite_eval_state.remaining = 2;
						break;
					}
					ppu->sprite_eval_state.remaining--;
					}
				else
				{
					ppu->sprite_eval_state.remaining--;
				}
			
			}
			// Sprite tile loading is normally done during dots 257..320, but we load all the data at once
			else if (ppu->cycles == 257)
			{
				ppu->OAMADDR = 0;
				ppu->sprite_zero_on_current_scanline = ppu->sprite_zero_on_next_scanline;

				// For every sprite to be drawn on the next scanline
				for (int i = 0; i < 8; i++)
				{
					// Read sprite data from secondary OAM
					uint8_t ypos = ppu->bus->secondary_OAM[4 * i];
					uint8_t tile_index = ppu->bus->secondary_OAM[4 * i + 1];
					ppu->sprite_attribute[i] = ppu->bus->secondary_OAM[4 * i + 2];
					ppu->sprite_xpos[i] = ppu->bus->secondary_OAM[4 * i + 3];

					// Calculate the address of the sprite in the pattern table
					uint16_t pattern_table_addr;
					if (ppu->PPUCTRL.flags.H) // 8x16 sprites
					{
						// Get fine y position within the tile
						uint8_t fine_y = (ppu->scanline - ypos) & 0x7;

						// Are we drawing the top or bottom tile of the sprite? 
						uint8_t tile_select = ((ppu->scanline - ypos) & 0x8) > 0;

						if (ppu->sprite_attribute[i] & 0x80) // vertical flip
						{
							fine_y = 7 - (int16_t)fine_y;
							tile_select = 1 - (int8_t)tile_select;
						}

						pattern_table_addr = (tile_index & 1) << 12 | (tile_index & 0xFE) << 4 | tile_select << 4 | fine_y;
					}
					else // 8x8 sprites
					{
						uint8_t fine_y = (ppu->scanline - ypos) & 0x7;
						if (ppu->sprite_attribute[i] & 0x80) // vertical flip
						{
							fine_y = 7 - (int16_t)fine_y;
						}
						pattern_table_addr = (ppu->PPUCTRL.flags.S << 12) | (tile_index << 4) | fine_y;
					}

					ppu->pt_sprite_low[i] = ppu_bus_read(ppu->bus, pattern_table_addr);
					ppu->pt_sprite_high[i] = ppu_bus_read(ppu->bus, pattern_table_addr | (1 << 3));
					if (i >= ppu->sprite_eval_state.secondary_oam_free_slot / 4)
					{
						ppu->pt_sprite_low[i] = 0;
						ppu->pt_sprite_high[i] = 0;
					}
				}
			}
		}
	}

	// TODO: figure out if VBL is cleared at dot 0 or dot 1. For now clear at dot 0
	if (ppu->scanline == -1 && ppu->cycles == 0)
	{
		ppu->PPUSTATUS.flags.V = 0;
	}
	else if (ppu->scanline == -1 && ppu->cycles == 1)
	{
		// Clear sprite overflow and sprite zero hit
		ppu->PPUSTATUS.flags.O = 0;
		ppu->PPUSTATUS.flags.S = 0;
	}

	// Start of vertical blanking
	if (ppu->scanline == 241 && ppu->cycles == 1)
	{
		if (!ppu->ppustatus_read_early && !ppu->ppustatus_read_late)
		{
			// Set vertical blank flag
			ppu->PPUSTATUS.flags.V = 1;
		}

		ppu->ppustatus_read_early = false;
		ppu->ppustatus_read_late = false;

		SwapFrameBuffers(ppu);
	}

	// Pull the nmi line low if we are in vertical blanking and nmi's are enabled
	ppu->nmi_line = (ppu->nmi_line << 1) | !(ppu->PPUCTRL.flags.V && ppu->PPUSTATUS.flags.V);

	// Emulate 9 ppu cycles of propagation delay TODO: figure out this exact timing
	const int delay = 9; 

	// NMI is triggered on falling edge. the line must be low for at least 2 ppu cycles, otherwise the cpu wont detect the edge
	if (((ppu->nmi_line & (0x7 << delay)) >> delay) == 0x4)
	{
		NMI(ppu->cpu);
	}

	// Increment cycles and scanline 

	ppu->cycles++;

	// cycle (340, -1) is skipped on odd frames and only if rendering is enabled
	if (ppu->cycles == 340 && ppu->scanline == -1 && ppu->oddframe && ((ppu->PPUMASK.reg & 0x18) != 0))
	{
		ppu->cycles++;
	}

	if (ppu->cycles >= 341)
	{
		ppu->cycles = 0;
		ppu->scanline++;
		if (ppu->scanline >= 261)
		{
			ppu->scanline = -1;
			ppu->oddframe = !ppu->oddframe;
			ppu->frame_count++;
		}
	}

	// Increment global cycle counter, for debugging
	ppu->total_cycles++;
}

void reset_2C02(State2C02* ppu)
{
	ppu->PPUCTRL.reg = 0;
	ppu->PPUMASK.reg = 0;

	ppu->w = 0;

	ppu->PPUSCROLL = 0;

	ppu->PPUDATA = 0;

	ppu->oddframe = 0;

	ppu->scanline = 0;
	ppu->cycles = 0;
}

void power_on_2C02(State2C02* ppu)
{
	ppu->PPUCTRL.reg = 0;
	ppu->PPUMASK.reg = 0;
	ppu->PPUSTATUS.reg = 0b10100000;
	ppu->OAMADDR = 0x00;

	ppu->w = 0;

	ppu->PPUSCROLL = 0;
	ppu->PPUADDR = 0;
	ppu->PPUDATA = 0;

	ppu->oddframe = 0;

	ppu->scanline = 0;
	ppu->cycles = 0;

	ppu->active_sprites = 0;

	ppu->total_cycles = 0;
	ppu->frame_count = 0;
	memset(ppu->pixels, 255, sizeof(ppu->pixels));
	ppu->back_buffer = 0;
	ppu->ppustatus_read_early = false;
	ppu->ppustatus_read_late = false;

	ppu->nmi_line = 0xFFFFFFFF;
}

void ppu_write(State2C02* ppu, uint16_t addr, uint8_t data)
{
	ppu->PPUSTATUS.reg = (ppu->PPUSTATUS.reg & 0xE0) | (data & 0x1F);
	switch (addr)
	{
	case 0x2000: // PPUCTRL
		ppu->PPUCTRL.reg = data;

		// Modify nametable select of temporary VRAM address 
		ppu->t &= 0b0111001111111111; // Clear bits 10 and 11
		ppu->t |= (uint16_t)ppu->PPUCTRL.flags.N << 10; // Set bits 10 and 11
		break;
	case 0x2001: // PPUMASK
		ppu->PPUMASK.reg = data;
		break;
	case 0x2003: // OAMADDR
		ppu->OAMADDR = data;
		break;
	case 0x2004: // OAMDATA
		// Only perform the write during vertical blanking or if rendering is disabled
		ppu->OAMDATA = data;
		if ((ppu->scanline >= 240 && ppu->scanline <= 260) || (!ppu->PPUMASK.flags.b && !ppu->PPUMASK.flags.s))
		{
			ppu->bus->OAM[ppu->OAMADDR] = ppu->OAMDATA;
			ppu->OAMADDR++;
		}
		break;
	case 0x2005: // PPUSCROLL
		ppu->PPUSCROLL = data;
		if (ppu->w == 0) // First write
		{
			ppu->t &= 0b0111111111100000; // Clear bits 0..4
			ppu->t |= data >> 3; // Sets bits 0..4 of t to significant 5 bits of data

			ppu->x = data & 0b00000111; // Set fine x scroll to least significant 3 bits of data
		}
		else if (ppu->w == 1) // Second write
		{
			ppu->t &= 0b0000110000011111; // Clear bits 5..9 and 12..14

			// and set them
			ppu->t |= ((uint16_t)data & 0b111) << 12;
			ppu->t |= ((uint16_t)data >> 3) << 5;
		}
		ppu->w = !ppu->w;
		break;
	case 0x2006: // PPUADDR
		ppu->PPUADDR = data;
		// PPU addresses are 14 bits, write 6 bits then 8 bits
		if (ppu->w == 0) // First write
		{
			// Clear bits 8..15 of t
			ppu->t &= 0x00FF;

			// Set bits 8..14
			ppu->t |= ((uint16_t)data & 0b111111) << 8;
			
		}
		else if (ppu->w == 1) // Second write 
		{
			// Clear bits 0..7
			ppu->t &= 0xFF00;

			// And set them
			ppu->t |= data;

			ppu->v = ppu->t; // Update current address
		}

		ppu->w = !ppu->w;
		break;
	case 0x2007: // PPUDATA
		ppu_bus_write(ppu->bus, ppu->v, data);
		ppu->v += (ppu->PPUCTRL.flags.I ? 32 : 1);
		break;
	}
}

uint8_t ppu_read(State2C02* ppu, uint16_t addr)
{
	switch (addr)
	{
	case 0x2002: // PPUSTATUS
		// Clear write toggle
		ppu->w = 0;

		if (ppu->cycles == 0 && ppu->scanline == 241)
		{
			ppu->ppustatus_read_early = true;
			ppu->PPUSTATUS.flags.V = 0;
		}
		else if ((ppu->cycles == 1 || ppu->cycles == 2) && ppu->scanline == 241)
		{
			ppu->ppustatus_read_late = true;
			ppu->PPUSTATUS.flags.V = 1;
		}

		uint8_t ret = ppu->PPUSTATUS.reg;

		// Clear vertical blank flag
		ppu->PPUSTATUS.flags.V = 0;

		return ret;
	case 0x2004: // OAMDATA
	{
		ppu->OAMDATA = ppu->bus->OAM[ppu->OAMADDR];
		// Do not increment OAMADDR during vertical blanking or forced blanking (ie if rendering is disabled)
		if (!(ppu->scanline >= 240 && ppu->scanline <= 260) && (ppu->PPUMASK.flags.b || ppu->PPUMASK.flags.s))
		{
			ppu->OAMADDR++;
		}
		return ppu->OAMDATA;
		break;
	}
	case 0x2007: // PPUDATA
	{
		// Reading pattern table or nametable data returns the contents of the internal buffer then updates it after
		if (ppu->v >= 0 && ppu->v < 0x3F00)
		{
			uint8_t ret = ppu->PPUDATA;
			ppu->PPUDATA = ppu_bus_read(ppu->bus, ppu->v);
			ppu->v += (ppu->PPUCTRL.flags.I ? 32 : 1);
			return ret;
		}
		// Reading palette data immediatly returns the requested data
		else if (ppu->v >= 0x3F00 && ppu->v < 0x4000)
		{
			// Mirrored nametable data is stored in internal buffer
			ppu->PPUDATA = ppu_bus_read(ppu->bus, 0x2000 | ppu->v & 0x0FFF);
			uint8_t ret = ppu_bus_read(ppu->bus, ppu->v);
			ppu->v += (ppu->PPUCTRL.flags.I ? 32 : 1);

			// Apply greyscale if enabled
			if (ppu->PPUMASK.flags.g)
			{
				ret &= 0x30;
			}

			return ret;
		}
		break;
	}
	}

	return 0;
}

uint8_t* get_framebuffer(State2C02* ppu)
{
	return &ppu->pixels[1 - ppu->back_buffer][0];
}