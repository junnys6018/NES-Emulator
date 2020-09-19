#ifndef RENDERER_H
#define RENDERER_H
#include "../Backend/6502.h"

void Renderer_Init();
void Renderer_Shutdown();

void Renderer_Draw(State6502* cpu);
void Renderer_SetPageView(uint8_t page);

// side = 0: left nametable
// side = 1: right nametable
void DrawPatternTable(int xoff, int yoff, int side);
void LoadPatternTable(uint8_t* table_data, int side, uint8_t palette[4]);

#endif