#include "NesStateView.h"
#include "Common.h"
#include <stdio.h>
#include <assert.h>

void DrawCPUStatus(int xoff, int yoff, State6502* cpu)
{
	int padding = GetWindowMetrics()->padding;

	SetTextOrigin(xoff + padding, yoff + padding);
	RenderText("Status:  ", white);
	SameLine();
	char flags[] = "NV-BDIZC";
	for (int i = 0; i < 8; i++)
	{
		// We are drawing the flags from high bits to low bits
		bool set = cpu->status.reg & (1 << (7 - i));
		RenderChar(flags[i], set ? green : red);
	}
	NewLine();
	// Draw Registers, PC and SP
	char line[32];

	sprintf(line, "A:  $%.2X", cpu->A);
	RenderText(line, white);

	sprintf(line, "X:  $%.2X", cpu->X);
	RenderText(line, white);

	sprintf(line, "Y:  $%.2X", cpu->Y);
	RenderText(line, white);

	sprintf(line, "PC: $%.4X", cpu->PC);
	RenderText(line, white);

	sprintf(line, "SP: $%.4X", 0x0100 | (uint16_t)cpu->SP);
	RenderText(line, white);

	// Total cycles
	sprintf(line, "cycles: %lli", cpu->total_cycles);
	RenderText(line, white);
}

void DrawStackView(int xoff, int yoff, State6502* cpu)
{
	int padding = GetWindowMetrics()->padding;
	int stack_size = (0xFF - cpu->SP) < 7 ? (0xFF - cpu->SP) : 7;

	SetTextOrigin(xoff + padding, yoff + padding);
	RenderText("Stack:", cyan);

	// Draw up to 7 lines of the stack
	for (int i = 0; i < stack_size; i++)
	{
		uint16_t addr = (cpu->SP + i + 1) | (1 << 8);
		uint8_t val = cpu_bus_read(cpu->bus, addr);
		char line[32];
		sprintf(line, "$%.4X: $%.2X", addr, val);

		RenderText(line, white);
	}
}

void DrawProgramView(int xoff, int yoff, State6502* cpu)
{
	int padding = GetWindowMetrics()->padding;

	SetTextOrigin(xoff + padding, yoff + padding);
	uint16_t PC = cpu->PC;
	int size;
	char line[128];
	for (int i = 0; i < 8; i++)
	{
		dissassemble(cpu, PC, &size, line);
		PC += size;
		RenderText(line, i == 0 ? cyan : white);
	}
}

void DrawPPUStatus(int xoff, int yoff, State2C02* ppu)
{
	int padding = GetWindowMetrics()->padding;
	Nes* nes = GetBoundNes();

	SetTextOrigin(xoff + padding, yoff + padding);
	RenderText("PPUCTRL   ($2000): ", white);
	SameLine();
	char flags[] = "VPHBSINN";
	for (int i = 0; i < 8; i++)
	{
		// We are drawing the flags from high bits to low bits
		bool set = ppu->PPUCTRL.reg & (1 << (7 - i));
		RenderChar(flags[i], set ? green : red);
	}
	NewLine();

	RenderText("PPUMASK   ($2001): ", white);
	SameLine();
	strcpy(flags, "BGRsbMmG");
	for (int i = 0; i < 8; i++)
	{
		bool set = ppu->PPUMASK.reg & (1 << (7 - i));
		RenderChar(flags[i], set ? green : red);
	}
	NewLine();

	RenderText("PPUSTATUS ($2002): ", white);
	SameLine();
	strcpy(flags, "VSO.....");
	for (int i = 0; i < 8; i++)
	{
		bool set = ppu->PPUSTATUS.reg & (1 << (7 - i));
		RenderChar(flags[i], set ? green : red);
	}
	NewLine();

	char line[64];
	sprintf(line, "OAMADDR   ($2003): $%.2X", ppu->OAMADDR);
	RenderText(line, white);

	sprintf(line, "OAMDATA   ($2004): $%.2X", ppu->OAMDATA);
	RenderText(line, white);

	sprintf(line, "PPUSCROLL ($2005): $%.2X", ppu->PPUSCROLL);
	RenderText(line, white);

	sprintf(line, "PPUADDR   ($2006): $%.2X", ppu->PPUADDR);
	RenderText(line, white);

	sprintf(line, "PPUDATA   ($2007): $%.2X", ppu->PPUDATA);
	RenderText(line, white);

	sprintf(line, "OAMDMA    ($4014): $%.2X", nes->cpu.OAMDMA);
	RenderText(line, white);

	SetTextOrigin(xoff + TextBounds("PPUMASK   ($2001): BGRsbMmG").w + 2 * padding, yoff + padding);
	RenderText("Internal Registers", cyan);

	sprintf(line, "v: $%.4X; t: $%.4X", ppu->v, ppu->t);
	RenderText(line, white);

	sprintf(line, "fine-x: %i", ppu->x);
	RenderText(line, white);

	sprintf(line, "Write Toggle (w): %i", ppu->w);
	RenderText(line, white);

	sprintf(line, "cycles: %i", ppu->cycles);
	RenderText(line, white);

	sprintf(line, "scanline: %i", ppu->scanline);
	RenderText(line, white);
}

void DrawPatternTable(int xoff, int yoff, int side, NameTableModel* nt)
{
	GLuint target = (side == 0 ? nt->left_nametable.handle : nt->right_nametable.handle);

	int len = GetWindowMetrics()->pattern_table_len;
	SDL_Rect dest = {.x = xoff, .y = yoff, .w = len, .h = len};
	SubmitTexturedQuadf(&dest, target);
}

void DrawPaletteData(int xoff, int yoff, PaletteDataModel pal)
{
	int padding = GetWindowMetrics()->padding;
	int len = GetWindowMetrics()->palette_visual_len;

	SDL_Rect rect = {.x = xoff, .y = yoff, .w = len, .h = len};
	for (int y = 0; y < 8; y++)
	{
		rect.x = xoff;
		for (int x = 0; x < 4; x++)
		{
			color c = PALETTE_MAP[pal.pal[y * 4 + x] & 0x3F];
			SubmitColoredQuad(&rect, c.r, c.g, c.b);
			rect.x += len;
		}
		rect.y += len + padding;
	}
}

// Draws the pattern table to an OpenGL texture
void RendererRasterizePatternTable(int side, NameTableModel* nt, PaletteDataModel pal)
{
	GLuint target = (side == 0 ? nt->left_nametable.handle : nt->right_nametable.handle);
	uint8_t* table_data = (side == 0 ? nt->left_nt_data : nt->right_nt_data);
	if (!table_data)
		return;
	uint8_t* pixels = malloc(128 * 128 * 3);
	assert(pixels);

	for (int y = 0; y < 128; y++)
	{
		for (int x = 0; x < 128; x++)
		{
			uint16_t fine_y = y % 8;
			uint8_t fine_x = 7 - (x % 8); // Invert x
			uint16_t tile_row = y / 8;
			uint16_t tile_col = x / 8;

			uint16_t table_addr = tile_row << 8 | tile_col << 4 | fine_y;

			uint8_t bit_mask = 1 << fine_x;

			uint8_t pal_low = (table_data[table_addr] & bit_mask) > 0;
			uint8_t pal_high = (table_data[table_addr | 1 << 3] & bit_mask) > 0;
			uint8_t palette_index = pal_low | (pal_high << 1);
			color c = PALETTE_MAP[pal.pal[palette_index]];

			pixels[3 * (y * 128 + x)] = c.r;
			pixels[3 * (y * 128 + x) + 1] = c.g;
			pixels[3 * (y * 128 + x) + 2] = c.b;
		}
	}
	glTextureSubImage2D(target, 0, 0, 0, 128, 128, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	free(pixels);
}

void DrawNESState(NameTableModel* nt, PaletteDataModel pal)
{
	WindowMetrics* wm = GetWindowMetrics();
	int xoff = wm->db_x;
	int yoff = wm->db_y + wm->menu_button_h;
	int padding = wm->padding;
	State6502* cpu = &GetBoundNes()->cpu;
	State2C02* ppu = &GetBoundNes()->ppu;

	DrawProgramView(xoff, yoff, cpu);
	xoff += TextBounds("$0000: ORA ($00,X)").w + padding;
	DrawStackView(xoff, yoff, cpu);
	xoff += TextBounds("$0000: $00 ").w + padding;
	DrawCPUStatus(xoff, yoff, cpu);
	DrawPPUStatus(wm->db_x, wm->db_y + wm->menu_button_h + TextHeight(8) + padding, ppu);

	int x = padding + wm->db_x;
	int y = wm->db_y + wm->menu_button_h + TextHeight(18) + padding;

	RendererRasterizePatternTable(0, nt, pal);
	RendererRasterizePatternTable(1, nt, pal);

	DrawPatternTable(x, y, 0, nt);
	DrawPatternTable(x + wm->pattern_table_len + padding, y, 1, nt);
	DrawPaletteData(x + 2 * wm->pattern_table_len + 2 * padding, y, pal);
}