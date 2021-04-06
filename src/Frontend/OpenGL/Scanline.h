#ifndef SCANLINE_H
#define SCANLINE_H
#include <glad/glad.h>

void InitScanlineEffect();
void ShutdownScanlineEffect();
GLuint ScanlineOnDraw(float time, GLuint source);

#endif