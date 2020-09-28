#include "2C02.h"
#include "Frontend/Renderer.h" // To upload pixel data to renderer

#include <stdio.h> // For printf
// TODO: Skip cycle (340,261) on odd frames
// TODO: Implment color emphasis and grey scale

// Maps a 6 bit HSV color into RGB
color PALETTE_MAP[64] =
{
	{ 84,  84,  84}, {  0,  30, 116}, {  8,  16, 144}, { 48,   0, 136}, { 68,   0, 100}, { 92,   0,  48}, { 84,   4,   0}, { 60,  24,   0}, { 32,  42,   0}, {  8,  58,   0}, {  0,  64,   0}, {  0,  60,   0}, {  0,  50,  60}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
	{152, 150, 152}, {  8,  76, 196}, { 48,  50, 236}, { 92,  30, 228}, {136,  20, 176}, {160,  20, 100}, {152,  34,  32}, {120,  60,   0}, { 84,  90,   0}, { 40, 114,   0}, {  8, 124,   0}, {  0, 118,  40}, {  0, 102, 120}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
	{236, 238, 236}, { 76, 154, 236}, {120, 124, 236}, {176,  98, 236}, {228,  84, 236}, {236,  88, 180}, {236, 106, 100}, {212, 136,  32}, {160, 170,   0}, {116, 196,   0}, { 76, 208,  32}, { 56, 204, 108}, { 56, 180, 204}, { 60,  60,  60}, {  0,   0,   0}, {  0,   0,   0},
	{236, 238, 236}, {168, 204, 236}, {188, 188, 236}, {212, 178, 236}, {236, 174, 236}, {236, 174, 212}, {236, 180, 176}, {228, 196, 144}, {204, 210, 120}, {180, 222, 120}, {168, 226, 144}, {152, 226, 180}, {160, 214, 228}, {160, 162, 160}, {  0,   0,   0}, {  0,   0,   0}
};

uint16_t coarseXinc(uint16_t v)
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

uint16_t fineYinc(uint16_t v)
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

void fetch_data_inc_v(State2C02* ppu)
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
		uint8_t attrib_tbl_byte = ppu_bus_read(ppu->bus, attrib_tbl_addr);
		uint8_t shift = 2 * ((ppu->v >> 1) & 0x01) + 4 * ((ppu->v >> 6) & 0x01);
		uint8_t palatteID = (attrib_tbl_byte >> shift) & 0x03;

		ppu->pa_latch_low = (palatteID & 0x01 ? 0xFF : 0x00);
		ppu->pa_latch_high = (palatteID & 0x02 ? 0xFF : 0x00);
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
			ppu->v = fineYinc(ppu->v);
		}
		else
		{
			ppu->v = coarseXinc(ppu->v);
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

void clock_2C02(State2C02* ppu)
{
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
				ppu->pt_shift_low <<= 1;
				ppu->pt_shift_high <<= 1;
				ppu->pa_shift_low <<= 1;
				ppu->pa_shift_high <<= 1;

				fetch_data_inc_v(ppu);
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
				fetch_data_inc_v(ppu);
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

				uint8_t pixel_low = ppu->pt_shift_low & bit_mask ? 1 : 0;
				uint8_t pixel_high = ppu->pt_shift_high & bit_mask ? 1 : 0;
				uint8_t shade = pixel_high << 1 | pixel_low;

				uint8_t pal_low = ppu->pa_shift_low & bit_mask ? 1 : 0;
				uint8_t pal_high = ppu->pa_shift_high & bit_mask ? 1 : 0;
				uint8_t palatte = pal_high << 1 | pixel_low;

				int index = ppu->scanline * 256 + ppu->cycles - 1;
				if (shade == 0) // Universal background color
				{
					ppu->pixels[index] = PALETTE_MAP[ppu_bus_read(ppu->bus, 0x3F00) & 0x3F];
				}
				else
				{
					uint16_t palatte_addr = 0x3F00 | palatte << 2 | shade;
					ppu->pixels[index] = PALETTE_MAP[ppu_bus_read(ppu->bus, palatte_addr) & 0x3F];
				}

				ppu->pt_shift_low <<= 1;
				ppu->pt_shift_high <<= 1;
				ppu->pa_shift_low <<= 1;
				ppu->pa_shift_high <<= 1;

				fetch_data_inc_v(ppu);
			}
			else if (ppu->cycles == 257)
			{
				// Copy horizontal data from t to v
				ppu->v = (ppu->v & 0x7BE0) | (ppu->t & ~0x7BE0);
			}
			else if (ppu->cycles >= 321 && ppu->cycles < 337)
			{
				fetch_data_inc_v(ppu);
			}
		}
	}
	//static int count = 0;
	//count++;

	if (ppu->scanline == -1 && ppu->cycles == 1)
	{
		//printf("Count: %i\n", count);
		ppu->PPUSTATUS.flags.V = 0;
	}

	if (ppu->scanline == 241 && ppu->cycles == 1)
	{
		//count = 0;
		//printf("Reset count\n");
		ppu->PPUSTATUS.flags.V = 1;
		if (ppu->PPUCTRL.flags.V)
		{
			NMI(ppu->cpu);
		}
		// Send pixel data to renderer
		LoadPixelDataToScreen(ppu->pixels);
	}

	ppu->cycles++;

	// Skip a cycle on odd frames
	if (ppu->cycles == 340 && ppu->scanline == -1 && ppu->oddframe)
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
		}
	}
}

void reset_2C02(State2C02* ppu)
{
	ppu->PPUCTRL.reg = 0;
	ppu->PPUMASK.reg = 0;

	ppu->w = 0;

	ppu->PPUSCROLL = 0;

	ppu->PPUDATA = 0;

	ppu->oddframe = 0;

	ppu->scanline = -1;
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

	ppu->scanline = -1;
	ppu->cycles = 0;
}

void write_ppu(State2C02* ppu, uint16_t addr, uint8_t data)
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
		printf("OAMADDR\n");
		break;
	case 0x2004: // OAMDATA
		printf("OAMDATA\n");
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
	case 0x4014: // OAMDMA
		break;
	}
}

uint8_t read_ppu(State2C02* ppu, uint16_t addr)
{
	switch (addr)
	{
	case 0x2002: // PPUSTATUS
		ppu->w = 0;
		uint8_t ret = ppu->PPUSTATUS.reg;
		ppu->PPUSTATUS.flags.V = 0;
		return ret;
	case 0x2004: // OAMDATA
		printf("OAMDATA\n");
		break;
	case 0x2007: // PPUDATA
	{
		if (ppu->v >= 0 && ppu->v < 0x3F00)
		{
			uint8_t ret = ppu->PPUDATA;
			ppu->PPUDATA = ppu_bus_read(ppu->bus, ppu->v);
			ppu->v += (ppu->PPUCTRL.flags.I ? 32 : 1);
			return ret;
		}
		else if (ppu->v >= 0x3F00 && ppu->v < 0x4000)
		{
			// Mirrored nametable data is stored in internal buffer
			ppu->PPUDATA = ppu_bus_read(ppu->bus, 0x2000 | ppu->v & 0x0FFF);
			uint8_t ret = ppu_bus_read(ppu->bus, ppu->v);
			ppu->v += (ppu->PPUCTRL.flags.I ? 32 : 1);
			return ret;
		}
		break;
	}
	}

	return 0;
}
