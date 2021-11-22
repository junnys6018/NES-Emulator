#include "Texture.h"
#include <stdlib.h>
#include <stdint.h>

void GenerateTexture(Texture2D* tex, int w, int h, void* pixels, GLenum blendmode, GLenum format)
{
	tex->w = w;
	tex->h = h;
	tex->format = format;

	glGenTextures(1, &tex->handle);
	glBindTexture(GL_TEXTURE_2D, tex->handle);

	glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, blendmode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, blendmode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void DeleteTexture(Texture2D* tex)
{
	glDeleteTextures(1, &tex->handle);
}

void ClearTexture(Texture2D* tex)
{
	int channels;
	if (tex->format == GL_RGB)
	{
		channels = 3;
	}
	else
	{
		channels = 4;
	}
	uint8_t* black = calloc(1, tex->w * tex->h * channels);
	if (black)
	{
		glTextureSubImage2D(tex->handle, 0, 0, 0, tex->w, tex->h, GL_RGB, GL_UNSIGNED_BYTE, black);
		free(black);
	}
}
