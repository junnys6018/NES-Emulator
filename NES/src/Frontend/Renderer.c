#include "Renderer.h"

#include <SDL.h> 
#include <stb_rect_pack.h>
#include <stb_truetype.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

static SDL_Color white = { 255,255,255 };
static SDL_Color cyan = { 78,201,176 };
static SDL_Color red = { 255,0,0 };
static SDL_Color green = { 0,255,0 };


typedef struct
{
	SDL_Window* win;
	SDL_Renderer* rend;

	stbtt_fontinfo info;
	unsigned char* fontdata;
	SDL_Texture* atlas;
	stbtt_packedchar chardata[96];
	float scale;
	int ascent, descent;

	int width, height;
	uint8_t page; // Page to view in memory
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

void Renderer_Init()
{
	rc.width = 1080;
	rc.height = 720;

	rc.page = 0;

	// TODO: only initialize video component and initialize other components when needed (eg audio)
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		SDL_Emit_Error("Could not initialize SDL");
	}

	rc.win = SDL_CreateWindow("NES", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, rc.width, rc.height, 0);
	if (!rc.win)
	{
		SDL_Emit_Error("Could not create window");
	}

	// creates a renderer to render our images 
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

	rc.scale = stbtt_ScaleForPixelHeight(&rc.info, 15);

	stbtt_pack_context spc;
	unsigned char* bitmap = malloc(512 * 512);
	stbtt_PackBegin(&spc, bitmap, 512, 512, 0, 1, NULL);
	stbtt_PackFontRange(&spc, rc.fontdata, 0, 15, 32, 96, rc.chardata);

	stbtt_PackEnd(&spc);

	stbi_write_png("out.png", 512, 512, 1, bitmap, 512);

	// Convert bitmap into SDL texture
	uint32_t* pixels = malloc(512 * 512 * sizeof(uint32_t));
	SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
	for (int i = 0; i < 512 * 512; i++) {
		pixels[i] = SDL_MapRGBA(format, bitmap[i], bitmap[i], bitmap[i], 0xff);
	}
	SDL_FreeFormat(format);

	rc.atlas = SDL_CreateTexture(rc.rend, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 512, 512);
	SDL_UpdateTexture(rc.atlas, NULL, pixels, 512 * sizeof(uint32_t));

	int lg;
	stbtt_GetFontVMetrics(&rc.info, &rc.ascent, &rc.descent, &lg);
}

void Renderer_Shutdown()
{
	// Cleanup
	SDL_DestroyRenderer(rc.rend);
	SDL_DestroyWindow(rc.win);
	SDL_Quit();

	free(rc.fontdata);
}

void Renderer_SetPageView(uint8_t page)
{
	rc.page = page;
}

void DrawMemoryView(SDL_Rect area, State6502* cpu);
void DrawCPUStatus(SDL_Rect area, State6502* cpu);
void DrawProgramView(SDL_Rect area, State6502* cpu);

void Renderer_Draw(State6502* cpu)
{
	// Clear screen to black
	SDL_SetRenderDrawColor(rc.rend, 32, 32, 32, 0);
	SDL_RenderClear(rc.rend);

	// Draw GUI
	SDL_SetRenderDrawColor(rc.rend, 16, 16, 16, 0);

	SDL_Rect r_MemoryView = { .x = 10,.y = 10,.w = rc.width / 2 - 15,.h = rc.height - 20 };
	SDL_RenderFillRect(rc.rend, &r_MemoryView);
	DrawMemoryView(r_MemoryView, cpu);

	SDL_Rect r_CPUStatus = { .x = rc.width / 2 + 5,.y = 10,.w = rc.width / 2 - 15,.h = rc.height / 2 - 15 };
	SDL_RenderFillRect(rc.rend, &r_CPUStatus);
	DrawCPUStatus(r_CPUStatus, cpu);

	SDL_Rect r_ProgramDissassembly = { .x = rc.width / 2 + 5,.y = rc.height / 2 + 5,.w = rc.width / 2 - 15,.h = rc.height / 2 - 15 };
	SDL_RenderFillRect(rc.rend, &r_ProgramDissassembly);
	DrawProgramView(r_ProgramDissassembly, cpu);

	// Swap framebuffers
	SDL_RenderPresent(rc.rend);
}

void DrawText(const char* text, SDL_Color c, int xoff, int yoff)
{
	yoff += roundf(rc.ascent * rc.scale);
	while (*text)
	{
		stbtt_packedchar* info = &rc.chardata[*text - 32];
		SDL_Rect src_rect = { info->x0, info->y0, info->x1 - info->x0, info->y1 - info->y0 };
		SDL_Rect dst_rect = { xoff + roundf(info->xoff), yoff + roundf(info->yoff), info->x1 - info->x0, info->y1 - info->y0 };

		SDL_SetTextureColorMod(rc.atlas, c.r, c.g, c.b);
		SDL_RenderCopy(rc.rend, rc.atlas, &src_rect, &dst_rect);
		xoff += roundf(info->xadvance);

		text++;
	}
}

void DrawChar(char glyph, SDL_Color c, int xoff, int yoff)
{
	yoff += roundf(rc.ascent * rc.scale);

	stbtt_packedchar* info = &rc.chardata[glyph - 32];
	SDL_Rect src_rect = { info->x0, info->y0, info->x1 - info->x0, info->y1 - info->y0 };
	SDL_Rect dst_rect = { xoff + roundf(info->xoff), yoff + roundf(info->yoff), info->x1 - info->x0, info->y1 - info->y0 };

	SDL_SetTextureColorMod(rc.atlas, c.r, c.g, c.b);
	SDL_RenderCopy(rc.rend, rc.atlas, &src_rect, &dst_rect);
	xoff += roundf(info->xadvance);
}

void DrawMemoryView(SDL_Rect area, State6502* cpu)
{
	DrawText("$ADDR  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F", cyan, area.x + 10, area.y + 10);

	// Draw 2 pages of memory
	for (int i = 0; i < 32; i++)
	{
		char line[128];
		uint16_t addr = i * 16 + ((uint16_t)rc.page << 8);
		uint8_t* m = cpu->bus->memory + addr;
		sprintf(line, "$%.4X  %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X", addr,
			m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11], m[12], m[13], m[14], m[15]);

		DrawText(line, white, area.x + 10, area.y + 35 + i * 20);
	}

	// Draw a box around the stack pointer
	SDL_Rect box = { .x = area.x + 70 + 27 * (cpu->SP & 0x000F),.y = area.y + 35 + 16 * 20 + 20 * ((cpu->SP & 0x00F0) >> 4),.w = 20,.h = 18 };
	SDL_SetRenderDrawColor(rc.rend, 0, 255, 0, 0);
	SDL_RenderDrawRect(rc.rend, &box);
	SDL_SetRenderDrawColor(rc.rend, 16, 16, 16, 0);
}

void DrawCPUStatus(SDL_Rect area, State6502* cpu)
{
	DrawText("Status: ", white, area.x + 10, area.y + 10);
	char flags[] = "NV-BDIZC";
	for (int i = 0; i < 8; i++)
	{
		// We are drawing the flags from high bits to low bits 
		bool set = cpu->status.reg & (1 << (7 - i));
		DrawChar(flags[i], set ? green : red, area.x + 80 + 10 * i, area.y + 10);
	}
	// Draw Registers, PC and SP
	char line[32];

	sprintf(line, "A: $%.2X", cpu->A);
	DrawText(line, white, area.x + 10, area.y + 30);

	sprintf(line, "X: $%.2X", cpu->X);
	DrawText(line, white, area.x + 10, area.y + 50);

	sprintf(line, "Y: $%.2X", cpu->Y);
	DrawText(line, white, area.x + 10, area.y + 70);

	sprintf(line, "PC: $%.4X", cpu->PC);
	DrawText(line, white, area.x + 10, area.y + 90);

	sprintf(line, "SP: $%.4X", 0x0100 | (uint16_t)cpu->SP);
	DrawText(line, white, area.x + 10, area.y + 110);

	// Total cycles
	sprintf(line, "Cycles: %i", cpu->total_cycles);
	DrawText(line, white, area.x + 10, area.y + 130);
}

void DrawProgramView(SDL_Rect area, State6502* cpu)
{
	uint16_t PC = cpu->PC;
	int size;
	for (int i = 0; i < 12; i++)
	{
		char* line = dissassemble(cpu, PC, &size);
		PC += size;

		DrawText(line, i == 0 ? cyan : white, area.x + 10, area.y + 10 + i * 20);

		free(line);
	}
}

