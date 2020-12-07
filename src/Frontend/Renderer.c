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
#include <math.h>

#include "timer.h"
#include "Gui.h"
#include "FileDialog.h"

SDL_Color white = { 255,255,255 };
SDL_Color cyan = { 78,201,176 };
SDL_Color red = { 255,0,0 };
SDL_Color green = { 0,255,0 };
SDL_Color blue = { 0,122,204 };
SDL_Color light_blue = { 28,151,234 };

///////////////////////////
//
// Section: Structs and enums
//
///////////////////////////

typedef enum
{
	TARGET_NES_STATE,
	TARGET_APU_OSC,
	TARGET_MEMORY,
	TARGET_ABOUT,
	TARGET_SETTINGS
} DrawTarget;

typedef struct
{
	int padding;

	int width, height; // width and height of window
	int db_x, db_y; // x and y offset of debug screen
	int db_w, db_h; // width and height of debug screen
	int nes_x, nes_y; // x and y offset of nes screen
	int nes_w, nes_h; // width and heigh of nes sceen

	// width and height of menu buttons
	float menu_button_w;
	int menu_button_h;

	int button_h; // height of normal buttons

	// width and height of pattern table visualisation
	int pattern_table_len;

	// length of each palette visualisation "box"
	int palette_visual_len;

	int apu_osc_height;
} WindowMetrics;

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
	int ascent, descent, line_gap;
	int y_advance;
	int font_size;
	int text_x, text_y, origin_x;

	// Window size metrics
	WindowMetrics wm;
	GuiMetrics gm;

	bool draw_debug_view;

	// Textures representing the pattern tables currently accessible on the PPU
	SDL_Texture* left_nametable;
	SDL_Texture* right_nametable;
	uint8_t* left_nt_data;
	uint8_t* right_nt_data;

	// Flags used to disable each of the channels, for debugging
	struct
	{
		bool SQ1;
		bool SQ2;
		bool TRI; 
		bool NOISE;
		bool DMC;
	} ch;

	// draw a grid over the nes screen if this is true
	bool draw_grid;

	// Pointer to the nes the renderer is drawing
	Nes* nes;

	// Pointer to the PPU's palette data
	uint8_t* palette;

	// Texture for PPU output
	SDL_Texture* nes_screen;

	// What screen we are currently drawing
	DrawTarget target;

	// Pointer to controller data
	Controller* controller;
} RendererContext;

static RendererContext rc;

///////////////////////////
//
// Section: Initialization and basic text drawing fuctions
//
///////////////////////////

void SDL_Emit_Error(const char* message)
{
	printf("[SDL ERROR] %s: %s\n", message, SDL_GetError());
}

void TTF_Emit_Error(const char* message)
{
	printf("[FONT ERROR]: %s\n", message);
}

void CalculateWindowMetrics(int w, int h)
{
	rc.draw_debug_view = true;

	rc.wm.width = w;
	rc.wm.height = h;

	rc.wm.padding = lroundf(0.0045f * (w + h));

	if ((float)w / h >= 256.0f / 240.0f)
	{
		rc.wm.nes_x = rc.wm.padding;
		rc.wm.nes_y = rc.wm.padding;

		rc.wm.nes_h = h - 2 * rc.wm.padding;
		rc.wm.nes_w = lroundf(rc.wm.nes_h * 256.0f / 240.0f); // maintain aspect ratio
	}
	else
	{
		rc.wm.nes_w = w - 2 * rc.wm.padding;
		rc.wm.nes_h = lroundf(rc.wm.nes_w * 240.0f / 256.0f);

		rc.wm.nes_x = rc.wm.padding;
		rc.wm.nes_y = (h - rc.wm.nes_h) / 2;
		rc.draw_debug_view = false;
	}

	rc.wm.db_x = 2 * rc.wm.padding + rc.wm.nes_w;
	rc.wm.db_y = rc.wm.padding;


	rc.wm.db_w = w - 3 * rc.wm.padding - rc.wm.nes_w;
	rc.wm.db_h = h - 2 * rc.wm.padding;

	const int min_db_w = 200, min_db_h = 400;
	if (rc.wm.db_w < min_db_w || rc.wm.db_h < min_db_h)
	{
		rc.draw_debug_view = false;

		// Center the nes screen to take up the space cleared from not drawing debug screen
		rc.wm.nes_x = (w - rc.wm.nes_w) / 2;
		rc.wm.nes_y = (h - rc.wm.nes_h) / 2;
	}

	rc.wm.button_h = lroundf(0.03f * h);
	rc.wm.pattern_table_len = lroundf(0.096f * (w + h));

	rc.wm.menu_button_w = (float)rc.wm.db_w / 5.0f;
	rc.wm.menu_button_h = lroundf(0.0406f * h);

	rc.wm.palette_visual_len = lroundf(0.004f * (w + h));

	rc.wm.apu_osc_height = lroundf(0.1355f * h);
}

void ClearTexture(SDL_Texture* tex)
{
	uint32_t fmt;
	int access, width, height;
	SDL_QueryTexture(tex, &fmt, &access, &width, &height);

	int pixel_size;
	if (fmt == SDL_PIXELFORMAT_RGB24)
	{
		pixel_size = 3;
	}
	else if (fmt == SDL_PIXELFORMAT_RGBA32)
	{
		pixel_size = 4;
	}
	else
	{
		assert(false);
	}

	void* clear_data = malloc(width * height * pixel_size);

	memset(clear_data, 0, width * height * pixel_size);
	SDL_UpdateTexture(tex, NULL, clear_data, width * pixel_size);

	free(clear_data);
}


void RendererInit(Controller* cont)
{
	const int starting_w = 1305, starting_h = 738;
	CalculateWindowMetrics(starting_w, starting_h);

	rc.controller = cont;

	// Create a window
	rc.win = SDL_CreateWindow("NES Emulator - By Jun Lim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, starting_w, starting_h, SDL_WINDOW_RESIZABLE);
	if (!rc.win)
	{
		SDL_Emit_Error("Could not create window");
	}

	// Create a renderer
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

	rc.font_size = 15; // distance from highest ascender to the lowest descender in pixels
	rc.scale = stbtt_ScaleForPixelHeight(&rc.info, (float)rc.font_size);

	stbtt_pack_context spc;
	unsigned char* bitmap = malloc(512 * 512);

	stbtt_PackBegin(&spc, bitmap, 512, 512, 0, 1, NULL);
	stbtt_PackFontRange(&spc, rc.fontdata, 0, (float)rc.font_size, 32, 96, rc.chardata);
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
	SDL_SetTextureBlendMode(rc.atlas, SDL_BLENDMODE_BLEND);
	
	free(bitmap);
	free(pixels);

	// Get font metrics
	stbtt_GetFontVMetrics(&rc.info, &rc.ascent, &rc.descent, &rc.line_gap);
	rc.y_advance = lroundf(rc.scale * (rc.ascent - rc.descent + rc.line_gap));
	rc.ascent = lroundf(rc.scale * rc.ascent);
	rc.descent = lroundf(rc.scale * rc.descent);
	rc.line_gap = lroundf(rc.scale * rc.line_gap);

	// Create nametable textures
	rc.left_nametable = SDL_CreateTexture(rc.rend, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, 128, 128);
	ClearTexture(rc.left_nametable);

	rc.right_nametable = SDL_CreateTexture(rc.rend, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, 128, 128);
	ClearTexture(rc.right_nametable);

	// And main screen 
	rc.nes_screen = SDL_CreateTexture(rc.rend, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 256, 240);
	ClearTexture(rc.nes_screen);

	// Enable sound channels
	rc.ch.SQ1 = true;
	rc.ch.SQ2 = true;
	rc.ch.TRI = true;
	rc.ch.NOISE = true;
	rc.ch.DMC = true;

	// GUI
	rc.gm.scroll_bar_width = 18;
	rc.gm.checkbox_size = 18;
	rc.gm.font_size = rc.font_size;
	rc.gm.padding = rc.wm.padding;
	GuiInit(rc.rend, &rc.gm);

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

	free(rc.fontdata);

	GuiShutdown();
}

void RenderChar(char glyph, SDL_Color c)
{
	stbtt_packedchar* info = &rc.chardata[glyph - 32];
	SDL_Rect src_rect = { info->x0, info->y0, info->x1 - info->x0, info->y1 - info->y0 };

	const int yoff = rc.text_y + rc.ascent + lroundf(info->yoff);
	const int xoff = rc.text_x + lroundf(info->xoff);
	SDL_Rect dst_rect = { xoff, yoff, info->x1 - info->x0, info->y1 - info->y0 };

	SDL_SetTextureColorMod(rc.atlas, c.r, c.g, c.b);
	SDL_RenderCopy(rc.rend, rc.atlas, &src_rect, &dst_rect);

	rc.text_x += lroundf(info->xadvance);
}

void RenderText(const char* text, SDL_Color c)
{
	rc.text_x = rc.origin_x;
	while (*text)
	{
		RenderChar(*text, c);
		text++;
	}
	rc.text_y += rc.y_advance;
}

Bounds TextBounds(const char* text)
{
	int sum = 0;
	while (*text)
	{
		sum += lroundf(rc.chardata[*text - 32].xadvance);
		text++;
	}

	Bounds b = { .w = sum,.h = rc.font_size };
	return b;
}

int TextHeight(int lines)
{
	return (lines - 1) * rc.y_advance + rc.ascent - rc.descent;
}

void SetTextOrigin(int xoff, int yoff)
{
	rc.text_x = xoff;
	rc.text_y = yoff;
	rc.origin_x = xoff;
}

void SameLine()
{
	rc.text_y -= rc.y_advance;
}

void NewLine()
{
	rc.text_y += rc.y_advance;
	rc.text_x = rc.origin_x;
}

///////////////////////////
//
// Section: Interfacing with the renderer
//
///////////////////////////

// Whenever a pattern table has been bank switched, the pointer the renderer has to the pattern table
// will be invalid, backend code will call this function to update that pointer to the new pattern table
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
}

void RendererBindNES(Nes* nes)
{
	rc.nes = nes;
	rc.palette = nes->ppu_bus.palette;
}

// The PPU will call this function whenever a new frame is ready
void SendPixelDataToScreen(color* pixels)
{
	color* dest;
	int pitch;
	SDL_LockTexture(rc.nes_screen, NULL, &dest, &pitch);

	memcpy(dest, pixels, 256 * 240 * 3);

	SDL_UnlockTexture(rc.nes_screen);
}

///////////////////////////
//
// Section: Rendering the screen
//
///////////////////////////

// This code does the actual work rasterizing the pattern table
void RendererRasterizePatternTable(int side)
{
	SDL_Texture* target = (side == 0 ? rc.left_nametable : rc.right_nametable);
	uint8_t* table_data = (side == 0 ? rc.left_nt_data : rc.right_nt_data);
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
			color c = PALETTE_MAP[rc.palette[palette_index]];

			pixels[3 * (y * 128 + x)] = c.r;
			pixels[3 * (y * 128 + x) + 1] = c.g;
			pixels[3 * (y * 128 + x) + 2] = c.b;
		}
	}
	SDL_UpdateTexture(target, NULL, pixels, 3 * 128);

	free(pixels);
}

void DrawMemoryView(int xoff, int yoff, State6502* cpu)
{
	static int cpu_addr_offset = 0;
	static int ppu_addr_offset = 0;

	// Scrollbars
	SDL_Rect span;
	span.x = xoff + rc.wm.padding / 2;
	span.y = yoff + rc.wm.padding + TextHeight(2);
	span.w = TextBounds("$ADDR  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F").w + rc.wm.padding + rc.gm.scroll_bar_width;
	span.h = TextHeight(13) + 2;

	GuiAddScrollBar("cpu memory", &span, &cpu_addr_offset, 0x80 - 13, 5);
	span.y = yoff + 2 * rc.wm.padding + TextHeight(15) + TextHeight(2);
	GuiAddScrollBar("test2", &span, &ppu_addr_offset, 0x400 - 13, 5);

	// Draw CPU memory
	SetTextOrigin(xoff + rc.wm.padding, yoff + rc.wm.padding);
	RenderText("CPU Memory", cyan);
	RenderText("$ADDR  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F", cyan);
	for (int i = 0; i < 13; i++)
	{
		char line[128];
		uint16_t addr = (i + cpu_addr_offset) * 16;
		uint8_t* m = cpu->bus->memory + addr;
		sprintf(line, "$%.4X  %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X", addr,
			m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11], m[12], m[13], m[14], m[15]);

		RenderText(line, white);
	}

	// Draw PPU memory
	SetTextOrigin(xoff + rc.wm.padding, yoff + 2 * rc.wm.padding + TextHeight(15));
	RenderText("PPU Memory", cyan);
	RenderText("$ADDR  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F", cyan);
	for (int i = 0; i < 13; i++)
	{
		char line[128];
		uint16_t addr = (i + ppu_addr_offset) * 16;
		uint8_t m[16];
		for (int i = 0; i < 16; i++)
		{
			m[i] = ppu_bus_peek(&rc.nes->ppu_bus, addr + i);
		}
		sprintf(line, "$%.4X  %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X", addr,
			m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11], m[12], m[13], m[14], m[15]);

		RenderText(line, white);
	}
}

void DrawCPUStatus(int xoff, int yoff, State6502* cpu)
{
	SetTextOrigin(xoff + rc.wm.padding, yoff + rc.wm.padding);
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
	int stack_size = (0xFF - cpu->SP) < 7 ? (0xFF - cpu->SP) : 7;

	SetTextOrigin(xoff + rc.wm.padding, yoff + rc.wm.padding);
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
	SetTextOrigin(xoff + rc.wm.padding, yoff + rc.wm.padding);
	uint16_t PC = cpu->PC;
	int size;
	for (int i = 0; i < 8; i++)
	{
		char* line = dissassemble(cpu, PC, &size);
		PC += size;

		RenderText(line, i == 0 ? cyan : white);

		free(line);
	}
}

void DrawPPUStatus(int xoff, int yoff, State2C02* ppu)
{
	SetTextOrigin(xoff + rc.wm.padding, yoff + rc.wm.padding);
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

	sprintf(line, "OAMDMA    ($4014): $%.2X", rc.nes->cpu.OAMDMA);
	RenderText(line, white);
	
	SetTextOrigin(xoff + TextBounds("PPUMASK   ($2001): BGRsbMmG").w + 2 * rc.wm.padding, yoff + rc.wm.padding);
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

void DrawPatternTable(int xoff, int yoff, int side)
{
	SDL_Texture* target = (side == 0 ? rc.left_nametable : rc.right_nametable);

	SDL_Rect dest = { .x = xoff,.y = yoff,.w = rc.wm.pattern_table_len,.h = rc.wm.pattern_table_len };
	SDL_RenderCopy(rc.rend, target, NULL, &dest);
}

void DrawPaletteData(int xoff, int yoff)
{
	int len = rc.wm.palette_visual_len;
	SDL_Rect rect = { .x = xoff,.y = yoff,.w = len,.h = len };
	for (int y = 0; y < 8; y++)
	{
		rect.x = xoff;
		for (int x = 0; x < 4; x++)
		{
			color c = PALETTE_MAP[rc.palette[y * 4 + x] & 0x3F];
			SDL_SetRenderDrawColor(rc.rend, c.r, c.g, c.b, 255);
			SDL_RenderFillRect(rc.rend, &rect);
			rect.x += len;
		}
		rect.y += len + rc.wm.padding;
	}
}

void DrawNESState()
{
	int yoff = rc.wm.db_y + rc.wm.menu_button_h;
	int xoff = rc.wm.db_x;
	DrawProgramView(xoff, yoff, &rc.nes->cpu);
	xoff += TextBounds("$0000: ORA ($00,X)").w + rc.wm.padding;
	DrawStackView(xoff, yoff, &rc.nes->cpu);
	xoff += TextBounds("$0000: $00 ").w + rc.wm.padding;
	DrawCPUStatus(xoff, yoff, &rc.nes->cpu);
	DrawPPUStatus(rc.wm.db_x, rc.wm.db_y + rc.wm.menu_button_h + TextHeight(8) + rc.wm.padding, &rc.nes->ppu);

	int x = rc.wm.padding + rc.wm.db_x;
	int y = rc.wm.db_y + rc.wm.menu_button_h + TextHeight(18) + rc.wm.padding;

	RendererRasterizePatternTable(0);
	RendererRasterizePatternTable(1);

	DrawPatternTable(x, y, 0);
	DrawPatternTable(x + rc.wm.pattern_table_len + rc.wm.padding, y, 1);
	DrawPaletteData(x + 2 * rc.wm.pattern_table_len + 2 * rc.wm.padding, y);
}

// A trigger function takes an audio buffer and returns an offset into that buffer
// when drawing a wave in DrawWaveform(), we start drawing from this offset.
// we do this to track the wave, otherwise the waveform will be jump around in the visualisation
// The pulse and triangle channels each require a different triggering algortihm, so we use a function pointer.
typedef int (*TRIGGER_FUNC)(AudioWindow* win);

// Find a rising edge
int pulse_trigger(AudioWindow* win)
{
	float max_slope = -100000.0f;
	int max_x = 0;
	for (int x = 0; x < 1024; x++)
	{
		float slope = win->buffer[(win->write_pos + 511 + x) % 2048] - win->buffer[(win->write_pos + 513 + x) % 2048];
		if (slope > max_slope)
		{
			max_slope = slope;
			max_x = x;
		}
	}

	return max_x;
}

// Find when the waveform crosses the x-axis from below
int triangle_trigger(AudioWindow* win)
{
	int zero_x = 0;
	float zero_sample = 100000.0f;
	for (int x = 0; x < 1024; x++)
	{
		float slope = win->buffer[(win->write_pos + 500 + x) % 2048] - win->buffer[(win->write_pos + 524 + x) % 2048];
		float abs_sample = win->buffer[(win->write_pos + 512 + x) % 2048];
		abs_sample = abs_sample < 0.0f ? -abs_sample : abs_sample;
		if (slope > 0 && abs_sample < zero_sample)
		{
			zero_sample = abs_sample;
			zero_x = x;
		}
	}

	return zero_x;
}

int no_trigger(AudioWindow* win)
{
	return 0;
}

// Visualise audio buffer as a waveform 
void DrawWaveform(SDL_Rect* rect, AudioWindow* win, float vertical_scale, TRIGGER_FUNC trigger)
{
	static SDL_Color waveform_colour = {247, 226, 64}; // yellow

	// Clear the part of the screen where we are drawing the waveform
	SDL_SetRenderDrawColor(rc.rend, 0, 0, 0, 255);
	SDL_RenderFillRect(rc.rend, rect);

	// Draw x and y axis
	SDL_SetRenderDrawColor(rc.rend, 128, 128, 128, 255);
	int mid_y = rect->y + rect->h / 2;
	int mid_x = rect->x + rect->w / 2;
	SDL_RenderDrawLine(rc.rend, rect->x, mid_y, rect->x + rect->w, mid_y);
	SDL_RenderDrawLine(rc.rend, mid_x, rect->y, mid_x, rect->y + rect->h);

	SDL_SetRenderDrawColor(rc.rend, waveform_colour.r, waveform_colour.g, waveform_colour.b, 255);

	// Find the offset into the audio buffer to start drawing from
	int trigger_x = trigger(win);

	// Allocate an array of points for each pixel on the x-axis
	SDL_Point* points = malloc(rect->w * sizeof(SDL_Point));

	// The audio buffer is a ring buffer, win->write_pos is the starting index into the ring buffer. The ring buffer contains 2048 samples
	int start_index = (win->write_pos + trigger_x) % 2048;
	float curr_index = 0.0f;

	// We want to visualise 1024 audio samples over rect->w pixels, so for each pixel we
	// increment by index_inc amount into the audio buffer. We are effectivly using a 
	// nearest neighbour filter to filter 1024 samples into rect->w samples
	float index_inc = 1024.0f / (float)rect->w;

	// For each pixel
	for (int i = 0; i < rect->w; i++)
	{
		points[i].x = rect->x + i;
		points[i].y = win->buffer[lroundf(curr_index + start_index) % 2048] * vertical_scale + mid_y;
		curr_index += index_inc;
	}
	SDL_RenderDrawLines(rc.rend, points, rect->w);

	free(points);
}

void DrawAPUOsc(int xoff, int yoff)
{
	if (GuiAddCheckbox("Square 1", xoff + rc.wm.padding, yoff + rc.wm.padding, &rc.ch.SQ1))
	{
		apu_channel_set(&rc.nes->apu, CHANNEL_SQ1, rc.ch.SQ1);
	}
	const int height = rc.wm.apu_osc_height;

	SDL_Rect rect = {.x = xoff + rc.wm.padding, .y = yoff + 2 * rc.wm.padding + rc.gm.checkbox_size, .w = rc.wm.db_w - 2 * rc.wm.padding, .h = height};
	DrawWaveform(&rect, &rc.nes->apu.SQ1_win, 350.0f, pulse_trigger);

	int curr_height = yoff + 3 * rc.wm.padding + rc.gm.checkbox_size + height;
	if (GuiAddCheckbox("Square 2", xoff + rc.wm.padding, curr_height, &rc.ch.SQ2))
	{
		apu_channel_set(&rc.nes->apu, CHANNEL_SQ2, rc.ch.SQ2);
	}
	curr_height += rc.wm.padding + rc.gm.checkbox_size;
	rect.y = curr_height;
	DrawWaveform(&rect, &rc.nes->apu.SQ2_win, 350.0f, pulse_trigger);

	curr_height += rc.wm.padding + height;
	if (GuiAddCheckbox("Triangle", xoff + rc.wm.padding, curr_height, &rc.ch.TRI))
	{
		apu_channel_set(&rc.nes->apu, CHANNEL_TRI, rc.ch.TRI);
	}
	curr_height += rc.wm.padding + rc.gm.checkbox_size;
	rect.y = curr_height;
	DrawWaveform(&rect, &rc.nes->apu.TRI_win, 200.0f, triangle_trigger);

	curr_height += rc.wm.padding + height;
	if (GuiAddCheckbox("Noise", xoff + rc.wm.padding, curr_height, &rc.ch.NOISE))
	{
		apu_channel_set(&rc.nes->apu, CHANNEL_NOISE, rc.ch.NOISE);
	}
	curr_height += rc.wm.padding + rc.gm.checkbox_size;
	rect.y = curr_height;
	DrawWaveform(&rect, &rc.nes->apu.NOISE_win, 200.0f, no_trigger);

	curr_height += rc.wm.padding + height;
	if (GuiAddCheckbox("DMC", xoff + rc.wm.padding, curr_height, &rc.ch.DMC))
	{
		apu_channel_set(&rc.nes->apu, CHANNEL_DMC, rc.ch.DMC);
	}

#if 0
	curr_height += rc.wm.padding + rc.gm.checkbox_size;
	SetTextOrigin(xoff + rc.wm.padding, curr_height);
	char buf[64];

	sprintf(buf, "Period: %i Timer: %i", rc.nes->apu.DMC_timer.period, rc.nes->apu.DMC_timer.counter);
	RenderText(buf, white);

	sprintf(buf, "sample addr: $%.4X sample len: %i", 0xC000 | (rc.nes->apu.DMC_ADDR << 6), (rc.nes->apu.DMC_LENGTH << 4) + 1);
	RenderText(buf, white);

	sprintf(buf, "output: %i remaining: %i", rc.nes->apu.DMC_LOAD_COUNTER, rc.nes->apu.DMC_memory_reader.bytes_remaining);
	RenderText(buf, white);
#endif
}

void SetAPUChannels()
{
	apu_channel_set(&rc.nes->apu, CHANNEL_SQ1, rc.ch.SQ1);
	apu_channel_set(&rc.nes->apu, CHANNEL_SQ2, rc.ch.SQ2);
	apu_channel_set(&rc.nes->apu, CHANNEL_TRI, rc.ch.TRI);
	apu_channel_set(&rc.nes->apu, CHANNEL_NOISE, rc.ch.NOISE);
	apu_channel_set(&rc.nes->apu, CHANNEL_DMC, rc.ch.DMC);
}

void DrawAbout(int xoff, int yoff)
{
	SetTextOrigin(xoff + rc.wm.padding, yoff + rc.wm.padding);
	RenderText("NES Emulator by Jun Lim", cyan);
	RenderText("Controls", white);
	RenderText("In Step Through Mode:", cyan);
	RenderText("Space - Emulate one CPU instruction", white);
	RenderText("f     - Emulate one frame", white);
	RenderText("p     - Emulate one master clock cycle", white);
	RenderText("While Running Emulation", cyan);
	RenderText("Z     - B button", white);
	RenderText("X     - A button", white);
	RenderText("Enter - Start", white);
	RenderText("Tab   - Select", white);
	RenderText("Arrow Keys for D-pad", white);
}

void ClearScreen()
{
	color* dest;
	int pitch;
	SDL_LockTexture(rc.nes_screen, NULL, &dest, &pitch);

	memset(dest, 0, 256 * 240 * sizeof(color));

	SDL_UnlockTexture(rc.nes_screen);
}

void DrawSettings(int xoff, int yoff)
{
	SDL_Rect span = {xoff + rc.wm.padding, yoff + rc.wm.padding, 0.09195f * rc.wm.width, rc.wm.button_h};

	if (GuiAddButton("Load ROM...", &span))
	{
		char file[256];
		if (OpenFileDialog(file, 256) == 0)
		{
			NESDestroy(rc.nes);
			if (NESInit(rc.nes, file) != 0)
			{
				// Failed to load rom
				rc.controller->mode = MODE_NOT_RUNNING;

				// ...so load a dummy one
				NESInit(rc.nes, NULL);
			}
			// Successfully loaded rom
			else if (rc.controller->mode == MODE_NOT_RUNNING)
			{
				rc.controller->mode = MODE_PLAY;
			}
			SetAPUChannels();
		}
		ClearScreen();
	}
	span.y = yoff + 2 * rc.wm.padding + rc.wm.button_h;
	span.w = 0.1494f * rc.wm.width;
	if (GuiAddButton(rc.controller->mode == MODE_PLAY ? "Step Through Emulation" : "Run Emulation", &span) && rc.controller->mode != MODE_NOT_RUNNING)
	{
		// Toggle between Step Through and Playing 
		rc.controller->mode = 1 - rc.controller->mode;
	}
	span.y += rc.wm.padding + rc.wm.button_h;
	span.w = 0.09195f * rc.wm.width;
	if (GuiAddButton("Reset", &span))
	{
		ClearScreen();
		NESReset(rc.nes);
		SetAPUChannels(); // Configue APU channels to current settings
	}
	span.y += rc.wm.padding + rc.wm.button_h;
	if (GuiAddButton("Save Game", &span))
	{

	}
	span.y += rc.wm.padding + rc.wm.button_h;
	if (GuiAddButton("Restore Game", &span))
	{

	}
	span.y += rc.wm.padding + rc.wm.button_h;

	GuiAddCheckbox("Draw Grid", span.x, span.y, &rc.draw_grid);
	span.y += rc.wm.padding + rc.gm.checkbox_size;
	
	static bool fullscreen = false;
	if (GuiAddCheckbox("Fullscreen", span.x, span.y, &fullscreen))
	{
		SDL_SetWindowFullscreen(rc.win, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
	}
	span.y += rc.wm.padding + rc.gm.checkbox_size;

	SetTextOrigin(xoff + rc.wm.padding, span.y);
	RenderText("ROM Infomation", cyan);
	Header header = rc.nes->cart.header;
	char buf[64];
	
	sprintf(buf, "Mapper        - %u", rc.nes->cart.mapperID);
	RenderText(buf, white);

	int PRG_banks = ((uint16_t)header.PRGROM_MSB << 8) | (uint16_t)header.PRGROM_LSB;
	sprintf(buf, "PRG ROM banks - %i [%iKB]", PRG_banks, PRG_banks * 16);
	RenderText(buf, white);

	int CHR_banks = ((uint16_t)header.CHRROM_MSB << 8) | (uint16_t)header.CHRROM_LSB;
	sprintf(buf, "CHR ROM banks - %i [%iKB]", CHR_banks, CHR_banks * 8);
	RenderText(buf, white);

	sprintf(buf, "PPU mirroring - %s", header.MirrorType ? "vertical" : "horizontal");
	RenderText(buf, white);

	sprintf(buf, "Battery       - %s", header.Battery ? "yes" : "no");
	RenderText(buf, white);

	sprintf(buf, "Trainer       - %s", header.Trainer ? "yes" : "no");
	RenderText(buf, white);

	sprintf(buf, "4-Screen      - %s", header.FourScreen ? "yes" : "no");
	RenderText(buf, white);
}

void RendererDraw()
{
	assert(rc.nes); // Nes must be bound for rendering

	int w, h;
	SDL_GetWindowSize(rc.win, &w, &h);
	if (w != rc.wm.width || h != rc.wm.height)
	{
		CalculateWindowMetrics(w, h);
	}

	// Clear screen to black
	SDL_SetRenderDrawColor(rc.rend, 32, 32, 32, 255);
	SDL_RenderClear(rc.rend);

	SDL_Rect r_NesView = {rc.wm.nes_x, rc.wm.nes_y, rc.wm.nes_w, rc.wm.nes_h};
	SDL_RenderCopy(rc.rend, rc.nes_screen, NULL, &r_NesView);
	
	// Draw 8x8 grid over nes screen for debugging
	if (rc.draw_grid)
	{
		for (int i = 0; i <= 32; i++)
		{
			if (i == 16)
				SDL_SetRenderDrawColor(rc.rend, 128, 128, 128, 255);
			else if (i % 2 == 0)
				SDL_SetRenderDrawColor(rc.rend, 64, 64, 64, 255);
			else
				SDL_SetRenderDrawColor(rc.rend, 32, 32, 32, 255);
			int x = rc.wm.nes_x + rc.wm.nes_w * i / 32;
			SDL_RenderDrawLine(rc.rend, x, rc.wm.nes_y, x, rc.wm.nes_y + rc.wm.nes_h);
		}
		for (int i = 0; i <= 30; i++)
		{
			if (i == 15)
				SDL_SetRenderDrawColor(rc.rend, 128, 128, 128, 255);
			else if (i % 2 == 0)
				SDL_SetRenderDrawColor(rc.rend, 64, 64, 64, 255);
			else
				SDL_SetRenderDrawColor(rc.rend, 32, 32, 32, 255);
			int y = rc.wm.nes_y + rc.wm.nes_w * i / 32;
			SDL_RenderDrawLine(rc.rend, rc.wm.nes_x, y, rc.wm.nes_x + rc.wm.nes_w, y);
		}
	}

	if (rc.draw_debug_view)
	{
		SDL_SetRenderDrawColor(rc.rend, 16, 16, 16, 255);
		SDL_Rect r_DebugView = {.x = rc.wm.db_x, .y = rc.wm.db_y, .w = rc.wm.db_w, .h = rc.wm.db_h};
		SDL_RenderFillRect(rc.rend, &r_DebugView);

		char* button_names[] = {"NES State", "APU Wave", "Memory", "About", "Settings"};
		DrawTarget targets[] = {TARGET_NES_STATE, TARGET_APU_OSC, TARGET_MEMORY, TARGET_ABOUT, TARGET_SETTINGS};
		int button_positions[6];

		button_positions[0] = rc.wm.db_x;
		button_positions[5] = rc.wm.db_x + rc.wm.db_w;
		for (int i = 1; i <= 4; i++)
		{
			button_positions[i] = lroundf(rc.wm.db_x + i * rc.wm.menu_button_w);
		}

		for (int i = 0; i < 5; i++)
		{
			SDL_Rect span;
			span.y = rc.wm.padding;
			span.h = rc.wm.menu_button_h;
			span.x = button_positions[i];
			span.w = button_positions[i + 1] - span.x;

			if (GuiAddButton(button_names[i], &span))
			{
				rc.target = targets[i];
			}
		}

		switch (rc.target)
		{
		case TARGET_NES_STATE:
			DrawNESState();
			break;
		case TARGET_APU_OSC:
			DrawAPUOsc(rc.wm.db_x, rc.wm.db_y + rc.wm.menu_button_h);
			break;
		case TARGET_MEMORY:
			DrawMemoryView(rc.wm.db_x, rc.wm.db_y + rc.wm.menu_button_h, &rc.nes->cpu);
			break;
		case TARGET_ABOUT:
			DrawAbout(rc.wm.db_x, rc.wm.db_y + rc.wm.menu_button_h);
			break;
		case TARGET_SETTINGS:
			DrawSettings(rc.wm.db_x, rc.wm.db_y + rc.wm.menu_button_h);
			break;
		}

		// Draw FPS
		SetTextOrigin(rc.wm.db_x + rc.wm.padding, rc.wm.db_y + rc.wm.db_h - rc.font_size - rc.wm.padding);
		char buf[64];
		sprintf(buf, "%.3f ms/frame", rc.controller->ms_per_frame);
		RenderText(buf, white);
	}

	GuiEndFrame();

	// Swap framebuffers
	SDL_RenderPresent(rc.rend);
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