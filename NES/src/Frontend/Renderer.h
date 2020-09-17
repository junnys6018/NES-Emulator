#ifndef RENDERER_H
#define RENDERER_H
#include "../Backend/6502.h"

void Renderer_Init();
void Renderer_Shutdown();

void Renderer_Draw(State6502* cpu);
void Renderer_SetPageView(uint8_t page);
void DrawPatternTable(int xoff, int yoff, int table_index);
void Renderer_SetPatternTableData(uint8_t* table_data);

#endif