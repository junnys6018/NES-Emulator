#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

GLuint BuildShader(const char* vertex_src, const char* fragment_src);
void DestroyShader(GLuint shader);

#endif
