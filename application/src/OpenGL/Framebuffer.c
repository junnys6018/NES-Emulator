#include "Framebuffer.h"
#include <stdio.h>

void GenerateFramebuffer(Framebuffer* fb, int w, int h)
{
	glGenFramebuffers(1, &fb->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fb->fbo);

	GenerateTexture(&fb->texture, w, h, NULL, GL_NEAREST, GL_RGB);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb->texture.handle, 0);

	glGenRenderbuffers(1, &fb->rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, fb->rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fb->rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("Framebuffer incomplete\n");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeleteFramebuffer(Framebuffer* fb)
{
	DeleteTexture(&fb->texture);
	glDeleteRenderbuffers(1, &fb->rbo);
	glDeleteFramebuffers(1, &fb->fbo);
}
