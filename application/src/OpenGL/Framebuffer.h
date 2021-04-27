#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H
#include <glad/glad.h>
#include "Texture.h"

typedef struct
{
	Texture2D texture;
	GLuint fbo, rbo;
} Framebuffer;

void GenerateFramebuffer(Framebuffer* fb, int w, int h);
void DeleteFramebuffer(Framebuffer* fb);

#endif
