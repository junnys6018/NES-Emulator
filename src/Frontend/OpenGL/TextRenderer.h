#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H
#include <stdint.h>
#include <SDL.h>

void InitTextRenderer(char fontfile[256], int font_size);
void ShutdownTextRenderer();

void SetTextOrigin(int xoff, int yoff);
void SameLine();
void NewLine();
void RenderChar(char glyph, SDL_Color col);
void RenderText(const char* text, SDL_Color col);

typedef struct
{
	int w, h;
} Bounds;
Bounds TextBounds(const char* text);
int TextHeight(int lines); // Returns the pixel height n lines of text will occupy

#endif 
