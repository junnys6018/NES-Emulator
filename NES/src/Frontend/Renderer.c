#include "Renderer.h"

#include <SDL.h> 
#include <stb_rect_pack.h>
#include <stb_truetype.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "timer.h"
#include "Gui.h"

SDL_Color white = { 255,255,255 };
SDL_Color cyan = { 78,201,176 };
SDL_Color red = { 255,0,0 };
SDL_Color green = { 0,255,0 };
SDL_Color blue = { 0,122,204 };
SDL_Color light_blue = { 28,151,234 };

typedef enum
{
	TARGET_NES_STATE,
	TARGET_APU_STATE,
	TARGET_MEMORY,
	TARGET_ABOUT,
	TARGET_SETTINGS
} DrawTarget;

typedef struct
{
	SDL_Window* win;
	SDL_Renderer* rend;

	// Font data
	stbtt_fontinfo info;
	uint8_t* fontdata;
	SDL_Texture* atlas;
	stbtt_packedchar chardata[96];
	float scale;
	int ascent, descent;

	// Width and height of the screen
	int width, height;
	uint8_t page; // Page to view in memory

	// Textures representing the pattern tables currently accessible on the PPU
	SDL_Texture* left_nametable;
	SDL_Texture* right_nametable;
	uint8_t* left_nt_data;
	uint8_t* right_nt_data;
	
	Nes* nes;

	// Pointer to the PPU's palette data
	uint8_t* palette;

	// Cached copy of the first background palette to detect changes in palette data
	uint8_t cached_bg_palette[4]; 

	// Texture for PPU output
	SDL_Texture* nes_screen;

	// What screen we are currently drawing
	DrawTarget target;

	bool overscan; // true: show overscan; false: hide overscan
} RendererContext;

static RendererContext rc;

void SDL_Emit_Error(const char* message)
{
	printf("[SDL ERROR] %s: %s\n", message, SDL_GetError());
}

void TTF_Emit_Error(const char* message)
{
	printf("[FONT ERROR]: %s\n", message);
}

void RendererInit()
{
	rc.width = 1298;
	rc.height = 740;

	rc.page = 0;

	// TODO: only initialize video component and initialize other components when needed (eg audio)
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		SDL_Emit_Error("Could not initialize SDL");
	}
	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
	// Create a window
	rc.win = SDL_CreateWindow("NES Emulator - By Jun Lim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, rc.width, rc.height, 0);
	if (!rc.win)
	{
		SDL_Emit_Error("Could not create window");
	}

	// Create a renderer to render our images 
	rc.rend = SDL_CreateRenderer(rc.win, -1, SDL_RENDERER_ACCELERATED);
	if (!rc.rend)
	{
		SDL_Emit_Error("Could not create renderer");
	}

	// Load the font
	FILE* file = fopen("Consola.ttf", "rb");
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	rc.fontdata = malloc(size);
	if (!rc.fontdata)
	{
		TTF_Emit_Error("Cound not allocate buffer for font file!");
	}
	fread(rc.fontdata, 1, size, file);
	fclose(file);


	if (!stbtt_InitFont(&rc.info, rc.fontdata, 0))
	{
		TTF_Emit_Error("Could not init font");
	}

	int font_size = 15; // distance from highest ascender to the lowest descender is 15 pixels
	rc.scale = stbtt_ScaleForPixelHeight(&rc.info, font_size);

	stbtt_pack_context spc;
	unsigned char* bitmap = malloc(512 * 512);
	stbtt_PackBegin(&spc, bitmap, 512, 512, 0, 1, NULL);
	stbtt_PackFontRange(&spc, rc.fontdata, 0, font_size, 32, 96, rc.chardata);

	stbtt_PackEnd(&spc);

	// Convert bitmap into SDL texture
	uint32_t* pixels = malloc(512 * 512 * sizeof(uint32_t));
	SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
	for (int i = 0; i < 512 * 512; i++)
	{
		pixels[i] = SDL_MapRGBA(format, 255, 255, 255, bitmap[i]);
	}
	SDL_FreeFormat(format);

	rc.atlas = SDL_CreateTexture(rc.rend, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 512, 512);
	SDL_UpdateTexture(rc.atlas, NULL, pixels, 512 * sizeof(uint32_t));

	int lg;
	stbtt_GetFontVMetrics(&rc.info, &rc.ascent, &rc.descent, &lg);

	free(bitmap);
	free(pixels);

	SDL_SetTextureBlendMode(rc.atlas, SDL_BLENDMODE_BLEND);

	// Create nametable textures
	rc.left_nametable = SDL_CreateTexture(rc.rend, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, 128, 128);
	rc.right_nametable = SDL_CreateTexture(rc.rend, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, 128, 128);

	// And main screen 
	rc.nes_screen = SDL_CreateTexture(rc.rend, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 256, 240);

	// GUI
	GuiInit(rc.rend);

	rc.target = TARGET_NES_STATE;
}

void RendererShutdown()
{
	// Cleanup
	SDL_DestroyTexture(rc.atlas);

	SDL_DestroyTexture(rc.left_nametable);
	SDL_DestroyTexture(rc.right_nametable);
	SDL_DestroyTexture(rc.nes_screen);

	SDL_DestroyRenderer(rc.rend);
	SDL_DestroyWindow(rc.win);

	SDL_Quit();

	free(rc.fontdata);

	GuiShutdown();
}

void RendererSetPageView(uint8_t page)
{
	rc.page = page;
}

void RenderChar(char glyph, SDL_Color c, int xoff, int yoff)
{
	yoff += roundf(rc.ascent * rc.scale);

	stbtt_packedchar* info = &rc.chardata[glyph - 32];
	SDL_Rect src_rect = { info->x0, info->y0, info->x1 - info->x0, info->y1 - info->y0 };
	SDL_Rect dst_rect = { xoff + roundf(info->xoff), yoff + roundf(info->yoff), info->x1 - info->x0, info->y1 - info->y0 };

	SDL_SetTextureColorMod(rc.atlas, c.r, c.g, c.b);
	SDL_RenderCopy(rc.rend, rc.atlas, &src_rect, &dst_rect);
}

void RenderText(const char* text, SDL_Color c, int xoff, int yoff)
{
	while (*text)
	{
		RenderChar(*text, c, xoff, yoff);
		stbtt_packedchar* info = &rc.chardata[*text - 32];
		xoff += roundf(info->xadvance);

		text++;
	}
}

int TextLen(const char* text)
{
	float sum = 0.0f;
	while (*text)
	{
		sum += rc.chardata[*text - 32].xadvance;
		text++;
	}

	return roundf(sum);
}

void DrawMemoryView(int xoff, int yoff, State6502* cpu)
{
	RenderText("$ADDR  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F", cyan, xoff + 10, yoff + 10);

	// Draw 2 pages of memory
	for (int i = 0; i < 32; i++)
	{
		char line[128];
		uint16_t addr = i * 16 + ((uint16_t)rc.page << 8);
		uint8_t* m = cpu->bus->memory + addr;
		sprintf(line, "$%.4X  %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X", addr,
			m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11], m[12], m[13], m[14], m[15]);

		RenderText(line, white, xoff + 10, yoff + 35 + i * 20);
	}
}

void DrawCPUStatus(int xoff, int yoff, State6502* cpu)
{
	RenderText("Status: ", white, xoff + 10, yoff + 10);
	char flags[] = "NV-BDIZC";
	for (int i = 0; i < 8; i++)
	{
		// We are drawing the flags from high bits to low bits 
		bool set = cpu->status.reg & (1 << (7 - i));
		RenderChar(flags[i], set ? green : red, xoff + TextLen("Status:  ") + 10 * i, yoff + 10);
	}
	// Draw Registers, PC and SP
	char line[32];

	sprintf(line, "A: $%.2X", cpu->A);
	RenderText(line, white, xoff + 10, yoff + 30);

	sprintf(line, "X: $%.2X", cpu->X);
	RenderText(line, white, xoff + 10, yoff + 50);

	sprintf(line, "Y: $%.2X", cpu->Y);
	RenderText(line, white, xoff + 10, yoff + 70);

	sprintf(line, "PC: $%.4X", cpu->PC);
	RenderText(line, white, xoff + 10, yoff + 90);

	sprintf(line, "SP: $%.4X", 0x0100 | (uint16_t)cpu->SP);
	RenderText(line, white, xoff + 10, yoff + 110);

	// Total cycles
	sprintf(line, "cycles: %lli", cpu->total_cycles);
	RenderText(line, white, xoff + 10, yoff + 130);
}

void DrawStackView(int xoff, int yoff, State6502* cpu)
{
	int stack_size = min(0xFF - cpu->SP, 7);

	RenderText("Stack:", cyan, xoff + 10, yoff + 10);

	// Draw up to 7 lines of the stack
	for (int i = 0; i < stack_size; i++)
	{
		uint16_t addr = (cpu->SP + i + 1) | (1 << 8);
		uint8_t val = cpu_bus_read(cpu->bus,addr);
		char line[32];
		sprintf(line, "$%.4X %.2X", addr, val);

		RenderText(line, white, xoff + 10, yoff + 30 + i * 20);
	}
}

void DrawProgramView(int xoff, int yoff, State6502* cpu)
{
	uint16_t PC = cpu->PC;
	int size;
	for (int i = 0; i < 8; i++)
	{
		char* line = dissassemble(cpu, PC, &size);
		PC += size;

		RenderText(line, i == 0 ? cyan : white, xoff + 10, yoff + 10 + i * 20);

		free(line);
	}
}

void DrawPPUStatus(int xoff, int yoff, State2C02* ppu)
{
	RenderText("PPUCTRL   ($2000): ", white, xoff + 10, yoff + 10);
	char flags[] = "VPHBSINN";
	for (int i = 0; i < 8; i++)
	{
		// We are drawing the flags from high bits to low bits 
		bool set = ppu->PPUCTRL.reg & (1 << (7 - i));
		RenderChar(flags[i], set ? green : red, xoff + TextLen("PPUCTRL   ($2000):  ") + 10 * i, yoff + 10);
	}

	RenderText("PPUMASK   ($2001): ", white, xoff + 10, yoff + 30);
	strcpy(flags, "BGRsbMmG");
	for (int i = 0; i < 8; i++)
	{
		bool set = ppu->PPUMASK.reg & (1 << (7 - i));
		RenderChar(flags[i], set ? green : red, xoff + TextLen("PPUMASK   ($2001):  ") + 10 * i, yoff + 30);
	}

	RenderText("PPUSTATUS ($2002): ", white, xoff + 10, yoff + 50);
	strcpy(flags, "VSO.....");
	for (int i = 0; i < 8; i++)
	{
		bool set = ppu->PPUSTATUS.reg & (1 << (7 - i));
		RenderChar(flags[i], set ? green : red, xoff + TextLen("PPUSTATUS ($2002):  ") + 10 * i, yoff + 50);
	}

	char line[64];
	sprintf(line, "OAMADDR   ($2003): $%.2X", ppu->OAMADDR);
	RenderText(line, white, xoff + 10, yoff + 70);

	sprintf(line, "OAMDATA   ($2004): $%.2X", ppu->OAMDATA);
	RenderText(line, white, xoff + 10, yoff + 90);

	sprintf(line, "PPUSCROLL ($2005): $%.2X", ppu->PPUSCROLL);
	RenderText(line, white, xoff + 10, yoff + 110);

	sprintf(line, "PPUADDR   ($2006): $%.2X", ppu->PPUADDR);
	RenderText(line, white, xoff + 10, yoff + 130);

	sprintf(line, "PPUDATA   ($2007): $%.2X", ppu->PPUDATA);
	RenderText(line, white, xoff + 10, yoff + 150);

	sprintf(line, "OAMDMA    ($4014): $%.2X", ppu->OAMDMA);
	RenderText(line, white, xoff + 10, yoff + 170);
	
	RenderText("Internal Registers", cyan, xoff + 280, yoff + 10);

	sprintf(line, "v: $%.4X; t: $%.4X", ppu->v, ppu->t);
	RenderText(line, white, xoff + 280, yoff + 30);

	sprintf(line, "fine-x: %i", ppu->x);
	RenderText(line, white, xoff + 280, yoff + 50);

	sprintf(line, "Write Toggle (w): %i", ppu->w);
	RenderText(line, white, xoff + 280, yoff + 70);

	// TODO: ppu->cycles and ppu->scanline indicate the next clock of the ppu, we want to render the current clock
	sprintf(line, "cycles: %i", ppu->cycles);
	RenderText(line, white, xoff + 280, yoff + 90);

	sprintf(line, "scanline: %i", ppu->scanline);
	RenderText(line, white, xoff + 280, yoff + 110);
}

void DrawPatternTable(int xoff, int yoff, float scale, int side)
{
	SDL_Texture* target = (side == 0 ? rc.left_nametable : rc.right_nametable);

	SDL_Rect dest = { .x = xoff,.y = yoff,.w = 128 * scale,.h = 128 * scale };
	SDL_RenderCopy(rc.rend, target, NULL, &dest);
}

void DrawPaletteData(int xoff, int yoff)
{
	SDL_Rect rect = { .x = xoff,.y = yoff,.w = 8,.h = 8 };
	for (int y = 0; y < 8; y++)
	{
		rect.x = xoff;
		for (int x = 0; x < 4; x++)
		{
			color c = PALETTE_MAP[rc.palette[y * 4 + x] & 0x3F];
			SDL_SetRenderDrawColor(rc.rend, c.r, c.g, c.b, 255);
			SDL_RenderFillRect(rc.rend, &rect);
			rect.x += 8;
		}
		rect.y += 14;
	}
}

// TODO: Make this more efficient, currently takes ~0.15ms to run this function
void RendererUpdatePatternTableTexture(int side)
{
	SDL_Texture* target = (side == 0 ? rc.left_nametable : rc.right_nametable);
	uint8_t* table_data = (side == 0 ? rc.left_nt_data : rc.right_nt_data);
	uint8_t* pixels = malloc(128 * 128 * 3);
	assert(pixels);

	for (int x = 0; x < 128; x++)
	{
		for (int y = 0; y < 128; y++)
		{
			uint16_t fine_y = y % 8;
			uint16_t fine_x = 7 - x % 8; // Invert x
			uint16_t tile_row = y / 8;
			uint16_t tile_col = x / 8;

			uint16_t table_addr = tile_row << 8 | tile_col << 4 | fine_y;

			uint8_t palette_index = (table_data[table_addr] & (1 << fine_x)) >> fine_x | table_data[table_addr | 1 << 3] & (1 << fine_x) >> (fine_x - 1);
			color c = PALETTE_MAP[rc.palette[palette_index]];

			pixels[3 * (y * 128 + x)] = c.r;
			pixels[3 * (y * 128 + x) + 1] = c.g;
			pixels[3 * (y * 128 + x) + 2] = c.b;
		}
	}
	SDL_UpdateTexture(target, NULL, pixels, 3 * 128);

	free(pixels);
}

void RendererSetPatternTable(uint8_t* table_data, int side)
{
	if (side == 0)
	{
		rc.left_nt_data = table_data;
	}
	else
	{
		rc.right_nt_data = table_data;
	}

	RendererUpdatePatternTableTexture(side);
}

void RendererBindNES(Nes* nes)
{
	rc.nes = nes;

	rc.palette = nes->ppu_bus.palette;
	for (int i = 0; i < 4; i++)
	{
		rc.cached_bg_palette[i] = rc.palette[i];
	}
}

void SendPixelDataToScreen(color* pixels)
{
	color* dest;
	int pitch;
	SDL_LockTexture(rc.nes_screen, NULL, &dest, &pitch);

	memcpy(dest, pixels, 256 * 240 * sizeof(color));

	SDL_UnlockTexture(rc.nes_screen);
}

void DrawNESState()
{
	DrawProgramView(788, 40, &rc.nes->cpu);
	DrawStackView(980, 40, &rc.nes->cpu);
	DrawCPUStatus(1100, 40, &rc.nes->cpu);
	DrawPPUStatus(788, 415, &rc.nes->ppu);

	int x = 798;
	int y = 220;
	float scale = 1.45f;

	// Check if palette has changed
	if (memcmp(rc.cached_bg_palette, rc.palette, 4) != 0)
	{
		memcpy(rc.cached_bg_palette, rc.palette, 4);
		if (rc.left_nt_data)
			RendererUpdatePatternTableTexture(0);
		if (rc.right_nt_data)
			RendererUpdatePatternTableTexture(1);
	}

	DrawPatternTable(x, y, scale, 0);
	DrawPatternTable(x + 128 * scale + 5, y, scale, 1);
	DrawPaletteData(x + 256 * scale + 10, y);
}

void DrawAbout(int xoff, int yoff)
{
	RenderText("NES Emulator by Jun Lim", cyan, xoff + 10, yoff + 10);
	RenderText("Controls", white, xoff + 10, yoff + 30);
	RenderText("Space - Emulate one CPU instruction", white, xoff + 10, yoff + 50);
	RenderText("f     - Emulate one frame", white, xoff + 10, yoff + 70);
	RenderText("p     - Emulate one master clock cycle", white, xoff + 10, yoff + 90);
}

void DrawSettings(int xoff, int yoff)
{
	SDL_Rect span = { xoff + 10, yoff + 10,120,20 };
	if (GuiAddButton("Load ROM", &span))
	{

	}
	if (GuiAddCheckbox("Overscan", xoff + 10, yoff + 40, &rc.overscan))
	{
		printf("cb %i\n", rc.overscan);
	}
	static bool running = false;
	span.y = yoff + 70; span.w = 200;
	if (GuiAddButton(running ? "Step Through Emulation" : "Run Emulation", &span))
	{
		running = !running;
	}
	span.y = yoff + 100; span.w = 120;
	if (GuiAddButton("Reset", &span))
	{

	}
	span.y = yoff + 130;
	if (GuiAddButton("Save Game", &span))
	{

	}
	span.y = yoff + 160;
	if (GuiAddButton("Restore Game", &span))
	{

	}

	RenderText("ROM Infomation", cyan, xoff + 10, yoff + 190);
	Header header = rc.nes->cart.header;
	char buf[64];

	uint16_t mapperID = ((uint16_t)header.MapperID3 << 8) | ((uint16_t)header.MapperID2 << 4) | (uint16_t)header.MapperID1;
	sprintf(buf, "Mapper        - %u", mapperID);
	RenderText(buf, white, xoff + 10, yoff + 210);
	
	int PRG_banks = ((uint16_t)header.PRGROM_MSB << 8) | (uint16_t)header.PRGROM_LSB;
	sprintf(buf, "PRG ROM banks - %i [%iKB]", PRG_banks, PRG_banks * 16);
	RenderText(buf, white, xoff + 10, yoff + 230);

	int CHR_banks = ((uint16_t)header.CHRROM_MSB << 8) | (uint16_t)header.CHRROM_LSB;
	sprintf(buf, "CHR ROM banks - %i [%iKB]", PRG_banks, PRG_banks * 8);
	RenderText(buf, white, xoff + 10, yoff + 250);

	sprintf(buf, "PPU mirroring - %s", header.MirrorType ? "vertical" : "horizontal");
	RenderText(buf, white, xoff + 10, yoff + 270);

	sprintf(buf, "Battery       - %s", header.Battery ? "yes" : "no");
	RenderText(buf, white, xoff + 10, yoff + 290);

	sprintf(buf, "Trainer       - %s", header.Trainer ? "yes" : "no");
	RenderText(buf, white, xoff + 10, yoff + 310);

	sprintf(buf, "4-Screen      - %s", header.FourScreen ? "yes" : "no");
	RenderText(buf, white, xoff + 10, yoff + 330);
}

void RendererDraw()
{
	assert(rc.nes); // Nes must be bound for rendering

	//timepoint beg, end;
	//GetTime(&beg);
	// Clear screen to black
	SDL_SetRenderDrawColor(rc.rend, 32, 32, 32, 255);
	SDL_RenderClear(rc.rend);

	// Draw GUI
	SDL_SetRenderDrawColor(rc.rend, 16, 16, 16, 255);

	SDL_Rect dest = { 10,10,768,720 };
	SDL_RenderCopy(rc.rend, rc.nes_screen, NULL, &dest);

	SDL_Rect r_DebugView = { .x = 788,.y = 10,.w = 500,.h = 720 };
	SDL_RenderFillRect(rc.rend, &r_DebugView);

	int width = 100;
	SDL_Rect span = { .x = 788,.y = 10,.w = width,.h = 30 };
	if (GuiAddButton("NES State", &span))
	{
		rc.target = TARGET_NES_STATE;
	}
	span.x += width;
	if (GuiAddButton("APU State", &span))
	{
		rc.target = TARGET_APU_STATE;
	}
	span.x += width;
	if (GuiAddButton("Memory", &span))
	{
		rc.target = TARGET_MEMORY;
	}
	span.x += width;
	if (GuiAddButton("About", &span))
	{
		rc.target = TARGET_ABOUT;
	}
	span.x += width;
	if (GuiAddButton("Settings", &span))
	{
		rc.target = TARGET_SETTINGS;
	}

	switch (rc.target)
	{
	case TARGET_NES_STATE:
		DrawNESState();
		break;
	case TARGET_APU_STATE:
		break;
	case TARGET_MEMORY:
		break;
	case TARGET_ABOUT:
		DrawAbout(788, 40);
		break;
	case TARGET_SETTINGS:
		DrawSettings(788, 40);
		break;
	}

	GuiEndFrame();

	// Swap framebuffers
	SDL_RenderPresent(rc.rend);

	//GetTime(&end);
	//float elapsed = GetElapsedTimeMilli(&beg, &end);
	//printf("Took %.3fms\n", elapsed);
}

#if 0
// For debugging
void DrawNametable(State2C02* ppu)
{
	color* pixels;
	color col[4] = { {0,0,0},{255,0,0,},{0,0,0,},{255,0,0} };
	int pitch;
	SDL_LockTexture(rc.nes_screen, NULL, &pixels, &pitch);

	memset(pixels, 255, 256 * 240 * 3);

	for (int x = 0; x < 32; x++)
	{
		for (int y = 0; y < 30; y++)
		{
			uint16_t nt_addr = 0x2000 | (y * 32 + x);
			uint8_t nt_byte = ppu_bus_read(ppu->bus, nt_addr);
			// Draw 8x8 tile
			for (int fine_y = 0; fine_y < 8; fine_y++)
			{
				uint16_t pt_addr = (uint16_t)(nt_byte << 4) | fine_y;
				uint8_t pt_byte_low = ppu_bus_read(ppu->bus, pt_addr);
				uint8_t pt_byte_high = ppu_bus_read(ppu->bus, pt_addr | 1 << 3);
				for (int fine_x = 0; fine_x < 8; fine_x++)
				{
					int yoff = y * 8 + fine_y;
					int xoff = x * 8 + 7 - fine_x;
					int index = yoff * 256 + xoff;
					int shade = (pt_byte_low & (1 << fine_x)) >> fine_x | pt_byte_high & (1 << fine_x) >> (fine_x - 1);
					pixels[index] = col[shade];
					if (fine_x == 0 || fine_y == 0)
					{
						pixels[index].r = 100;
						pixels[index].g = 100;
						pixels[index].b = 100;
					}
				}
			}
		}
	}

	SDL_UnlockTexture(rc.nes_screen);
}
#endif