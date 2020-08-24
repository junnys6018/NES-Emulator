#include "Renderer.h"

#include <SDL.h> 
#include <SDL_ttf.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

static SDL_Color white = { 255,255,255 };
static SDL_Color cyan = { 78,201,176 };
static SDL_Color red = { 255,0,0 };
static SDL_Color green = { 0,255,0 };

typedef struct
{
	SDL_Window* win;
	SDL_Renderer* rend;
	TTF_Font* font;

	int width, height;
	uint8_t page; // Page to view in memory
} RendererContext;

static RendererContext rc;

void TTF_Emit_Error(const char* message)
{
	printf("[TTF ERROR] %s: %s\n", message, TTF_GetError());
}

void SDL_Emit_Error(const char* message)
{
	printf("[SDL ERROR] %s: %s\n", message, SDL_GetError());
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

	if (TTF_Init() != 0)
	{
		TTF_Emit_Error("Could not initialize SDL_TTF");
	}

	rc.font = TTF_OpenFont("Consola.ttf", 16);
	if (!rc.font)
	{
		TTF_Emit_Error("Cant Open Font");
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
}

void Renderer_Shutdown()
{
	// Cleanup
	TTF_CloseFont(rc.font);
	TTF_Quit();

	SDL_DestroyRenderer(rc.rend);
	SDL_DestroyWindow(rc.win);
	SDL_Quit();
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
	SDL_Surface* s_line = TTF_RenderUTF8_Blended(rc.font, text, c);
	SDL_Texture* t_line = SDL_CreateTextureFromSurface(rc.rend, s_line);

	SDL_Rect rect = { .x = xoff,.y = yoff,.w = s_line->w,.h = s_line->h };
	SDL_RenderCopy(rc.rend, t_line, NULL, &rect);

	SDL_FreeSurface(s_line);
	SDL_DestroyTexture(t_line);
}

void DrawChar(char glyph, SDL_Color c, int xoff, int yoff)
{
	SDL_Surface* s_glyph = TTF_RenderGlyph_Blended(rc.font, glyph, c);
	SDL_Texture* t_glyph = SDL_CreateTextureFromSurface(rc.rend, s_glyph);

	SDL_Rect rect = { .x = xoff,.y = yoff,.w = s_glyph->w,.h = s_glyph->h };
	SDL_RenderCopy(rc.rend, t_glyph, NULL, &rect);

	SDL_FreeSurface(s_glyph);
	SDL_DestroyTexture(t_glyph);
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

