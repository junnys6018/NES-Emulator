#ifndef RENDERER_H
#define RENDERER_H
#include "Backend/nes.h"
#include "Controller.h"
#include <SDL.h>

void InitRenderer(Controller* cont);
void RendererShutdown();
void RendererDraw();

void GetWindowSize(int* w, int* h);

// side = 0: left nametable; side = 1: right nametable
void RendererSetPatternTable(uint8_t* table_data, int side);
void RendererBindNES(Nes* nes);
void SendPixelDataToScreen(color* pixels);
#endif