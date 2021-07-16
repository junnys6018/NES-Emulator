#include "TextRenderer.h"
#include "BatchRenderer.h"
#include "Texture.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <stb_rect_pack.h>
#include <stb_truetype.h>
#include <glad/glad.h>

typedef struct
{
	// Font data
	stbtt_fontinfo info;
	uint8_t* fontdata;
	Texture2D atlas;
	stbtt_packedchar chardata[96];
	float scale;
	int ascent, descent, line_gap;
	int y_advance;
	int font_size;
	int text_x, text_y, origin_x;
} Context;

static Context c;

void InitTextRenderer(char fontfile[256], int font_size)
{
	// Load the font file
	FILE* file = fopen(fontfile, "rb");
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	c.fontdata = malloc(size);
	if (!c.fontdata)
	{
		printf("Cound not allocate buffer for font file!");
	}
	fread(c.fontdata, 1, size, file);
	fclose(file);

	if (!stbtt_InitFont(&c.info, c.fontdata, 0))
	{
		printf("Could not init font");
	}

	c.font_size = font_size; // distance from highest ascender to the lowest descender in pixels
	c.scale = stbtt_ScaleForPixelHeight(&c.info, (float)c.font_size);

	stbtt_pack_context spc;
	unsigned char* bitmap = malloc(512 * 512);

	stbtt_PackBegin(&spc, bitmap, 512, 512, 0, 1, NULL);
	stbtt_PackFontRange(&spc, c.fontdata, 0, (float)c.font_size, 32, 96, c.chardata);
	stbtt_PackEnd(&spc);

	// Convert bitmap into OpenGL texture
	uint8_t* pixels = malloc(512 * 512 * 4);
	int j = 0;
	for (int i = 0; i < 512 * 512 * 4; i += 4)
	{
		pixels[i + 0] = 255;
		pixels[i + 1] = 255;
		pixels[i + 2] = 255;
		pixels[i + 3] = bitmap[j++];
	}


	GenerateTexture(&c.atlas, 512, 512, pixels, GL_LINEAR, GL_RGBA);

	free(bitmap);
	free(pixels);

	// Get font metrics
	stbtt_GetFontVMetrics(&c.info, &c.ascent, &c.descent, &c.line_gap);
	c.y_advance = lroundf(c.scale * (c.ascent - c.descent + c.line_gap));
	c.ascent = lroundf(c.scale * c.ascent);
	c.descent = lroundf(c.scale * c.descent);
	c.line_gap = lroundf(c.scale * c.line_gap);
}

void ShutdownTextRenderer()
{
	free(c.fontdata);
	DeleteTexture(&c.atlas);
}

void SetTextOrigin(int xoff, int yoff)
{
	c.text_x = xoff;
	c.text_y = yoff;
	c.origin_x = xoff;
}

void SameLine()
{
	c.text_y -= c.y_advance;
}

void NewLine()
{
	c.text_y += c.y_advance;
	c.text_x = c.origin_x;
}

void RenderChar(char glyph, SDL_Color col)
{
	stbtt_packedchar* info = &c.chardata[glyph - 32];
	SDL_Rect src = { info->x0, info->y0, info->x1 - info->x0, info->y1 - info->y0 };

	const int yoff = c.text_y + c.ascent + lroundf(info->yoff);
	const int xoff = c.text_x + lroundf(info->xoff);
	SDL_Rect dst_rect = { xoff, yoff, info->x1 - info->x0, info->y1 - info->y0 };

	SubmitTexturedColoredQuad(&dst_rect, c.atlas.handle, src.x / 512.0f, src.y / 512.0f, src.w / 512.0f, src.h / 512.0f, col.r, col.g, col.b);

	c.text_x += lroundf(info->xadvance);
}

void RenderText(const char* text, SDL_Color col)
{
	c.text_x = c.origin_x;
	while (*text)
	{
		RenderChar(*text, col);
		text++;
	}
	c.text_y += c.y_advance;
}

Bounds TextBounds(const char* text)
{
	int sum = 0;
	while (*text)
	{
		sum += lroundf(c.chardata[*text - 32].xadvance);
		text++;
	}

	Bounds b = {.w = sum, .h = c.font_size};
	return b;
}

int TextHeight(int lines)
{
	return (lines - 1) * c.y_advance + c.ascent - c.descent;
}
