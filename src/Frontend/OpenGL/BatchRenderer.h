#ifndef BATCH_RENDERER_H
#define BATCH_RENDERER_H
#include <glad/glad.h>

void InitBatchRenderer();
void ShutdownBatchRenderer();

void BeginBatch();
void EndBatch();
void SubmitColoredQuad(int x, int y, int w, int h, int r, int g, int b);
void SubmitTexturedQuad(int x, int y, int w, int h, GLuint texture, float tx, float ty, float tw, float th);

#endif
