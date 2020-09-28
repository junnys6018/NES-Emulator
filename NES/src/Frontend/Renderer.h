#ifndef RENDERER_H
#define RENDERER_H
#include "Backend/6502.h"
#include "Backend/2C02.h"

void RendererInit();
void RendererSetPaletteData(uint8_t* palette);
void RendererShutdown();

void RendererDraw(State6502* cpu, State2C02* ppu);
void RendererSetPageView(uint8_t page);

// side = 0: left nametable; side = 1: right nametable
void LoadPatternTable(uint8_t* table_data, int side, int palette_index);

void LoadPixelDataToScreen(color* pixels);

void DrawNametable(State2C02* ppu);

#endif