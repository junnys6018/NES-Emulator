#ifndef TEXTURE_H
#define TEXTURE_H
#include <glad/glad.h>

typedef struct
{
	GLuint handle;
	int w, h;
	GLenum format;
} Texture2D;

void GenerateTexture(Texture2D* tex, int w, int h, void* pixels, GLenum blendmode, GLenum format);
void DeleteTexture(Texture2D* tex);
void ClearTexture(Texture2D* tex);

#endif
