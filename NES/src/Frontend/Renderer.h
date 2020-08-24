#ifndef RENDERER_H
#define RENDERER_H
#include "../6502.h"

void Renderer_Init();
void Renderer_Shutdown();

void Renderer_Draw(State6502* cpu);
void Renderer_SetPageView(uint8_t page);


#endif // !RENDERER_H
