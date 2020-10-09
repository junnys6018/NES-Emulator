#ifndef RENDERER_H
#define RENDERER_H
#include "Backend/nes.h"
#include "Controller.h"
#include <SDL.h>

extern SDL_Color white;
extern SDL_Color cyan;
extern SDL_Color red;
extern SDL_Color green;
extern SDL_Color blue;
extern SDL_Color light_blue;

void RendererInit(Controller* cont);
void RendererShutdown();
void RendererDraw();

void SetTextOrigin(int xoff, int yoff);
void SameLine();
void NewLine();
void RenderChar(char glyph, SDL_Color c);
void RenderText(const char* text, SDL_Color c);

typedef struct
{
	int w, h;
} Bounds;
Bounds TextBounds(const char* text);
int TextHeight(int lines); // Returns the pixel height n lines of text will occupy 

// side = 0: left nametable; side = 1: right nametable
void RendererSetPatternTable(uint8_t* table_data, int side);
void RendererBindNES(Nes* nes);
void SendPixelDataToScreen(color* pixels);
#endif