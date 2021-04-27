#ifndef LINE_RENDERER_H
#define LINE_RENDERER_H
#include <stdint.h>

void InitLineRenderer();
void ShutdownLineRenderer();

void BeginLines();
void EndLines();
void SubmitLine(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b);

#endif
