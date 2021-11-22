#ifndef BATCH_RENDERER_H
#define BATCH_RENDERER_H
#include <glad/glad.h>
#include <SDL.h>

void InitBatchRenderer();
void ShutdownBatchRenderer();

void BeginBatch();
void EndBatch();
void SubmitTexturedColoredQuad(SDL_Rect* span, GLuint texture, float tx, float ty, float tw, float th, uint8_t r, uint8_t g, uint8_t b);

static inline void SubmitColoredQuad(SDL_Rect* span, uint8_t r, uint8_t g, uint8_t b)
{
	SubmitTexturedColoredQuad(span, 0, 0.0f, 0.0f, 0.0f, 0.0f, r, g, b);
}

static inline void SubmitTexturedQuadp(SDL_Rect* span, GLuint texture, float tx, float ty, float tw, float th)
{
	SubmitTexturedColoredQuad(span, texture, tx, ty, tw, th, 255, 255, 255);
}

static inline void SubmitTexturedQuadf(SDL_Rect* span, GLuint texture)
{
	SubmitTexturedColoredQuad(span, texture, 0.0f, 0.0f, 0.0f, 0.0f, 255, 255, 255);
}
#endif
