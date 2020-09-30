#ifndef RENDERER_H
#define RENDERER_H
#include "Backend/nes.h"
#include <SDL.h>

extern SDL_Color white;
extern SDL_Color cyan;
extern SDL_Color red;
extern SDL_Color green;
extern SDL_Color blue;
extern SDL_Color light_blue;

void RendererInit();
void RendererShutdown();
void RendererDraw();

void RenderText(const char* text, SDL_Color c, int xoff, int yoff);
int TextLen(const char* text);

// side = 0: left nametable; side = 1: right nametable
void RendererSetPatternTable(uint8_t* table_data, int side);
void RendererBindNES(Nes* nes);
void SendPixelDataToScreen(color* pixels);
#endif