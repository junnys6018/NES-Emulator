#include "MemoryView.h"
#include "Common.h"
#include <stdio.h>

void DrawMemoryView()
{
	WindowMetrics* wm = GetWindowMetrics();
	int padding = wm->padding;
	int xoff = wm->db_x;
	int yoff = wm->db_y + wm->menu_button_h;
	GuiMetrics* gm = GetGuiMetrics();
	Nes* nes = GetApplicationNes();

	static int cpu_addr_offset = 0;
	static int ppu_addr_offset = 0;

	// Scrollbars
	SDL_Rect span;
	span.x = xoff + padding / 2;
	span.y = yoff + padding + TextHeight(2);
	span.w = TextBounds("$ADDR  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F").w + padding + gm->scroll_bar_width;
	span.h = TextHeight(13) + 2;

	GuiAddScrollBar("cpu memory", &span, &cpu_addr_offset, 0x80 - 13, 5);
	span.y = yoff + 2 * padding + TextHeight(15) + TextHeight(2);
	GuiAddScrollBar("test2", &span, &ppu_addr_offset, 0x400 - 13, 5);

	// Draw CPU memory
	SetTextOrigin(xoff + padding, yoff + padding);
	RenderText("CPU Memory", cyan);
	RenderText("$ADDR  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F", cyan);
	for (int i = 0; i < 13; i++)
	{
		char line[128];
		uint16_t addr = (i + cpu_addr_offset) * 16;
		uint8_t* m = nes->cpu.bus->memory + addr;
		sprintf(line, "$%.4X  %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X", addr,
				m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11], m[12], m[13], m[14], m[15]);

		RenderText(line, white);
	}

	// Draw PPU memory
	SetTextOrigin(xoff + padding, yoff + 2 * padding + TextHeight(15));
	RenderText("PPU Memory", cyan);
	RenderText("$ADDR  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F", cyan);
	for (int i = 0; i < 13; i++)
	{
		char line[128];
		uint16_t addr = (i + ppu_addr_offset) * 16;
		uint8_t m[16];
		for (int i = 0; i < 16; i++)
		{
			m[i] = ppu_bus_peek(&nes->ppu_bus, addr + i);
		}
		sprintf(line, "$%.4X  %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X", addr,
				m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11], m[12], m[13], m[14], m[15]);

		RenderText(line, white);
	}
}
