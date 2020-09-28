#ifndef RENDERER_H
#define RENDERER_H
#include "Backend/nes.h"

void RendererInit();
void RendererShutdown();

void RendererDraw();
void RendererSetPageView(uint8_t page);

// side = 0: left nametable; side = 1: right nametable
void RendererSetPatternTable(uint8_t* table_data, int side);
void RendererBindNES(Nes* nes);

void SendPixelDataToScreen(color* pixels);
#endif