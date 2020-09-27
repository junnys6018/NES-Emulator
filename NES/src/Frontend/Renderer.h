#ifndef RENDERER_H
#define RENDERER_H
#include "Backend/6502.h"
#include "Backend/2C02.h"

void Renderer_Init();
void Renderer_Shutdown();

void Renderer_Draw(State6502* cpu, State2C02* ppu);
void Renderer_SetPageView(uint8_t page);

// side = 0: left nametable; side = 1: right nametable
void LoadPatternTable(uint8_t* table_data, int side, int palette_index);

void LoadPixelDataToScreen(color* pixels);

void DrawNametable(State2C02* ppu);

#endif