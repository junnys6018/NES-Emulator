#include "Shader.h"
#include <stdlib.h>
#include <stdio.h>

GLuint CompileShader(const char* src, GLenum shader_type)
{
	GLuint shader = glCreateShader(shader_type);

	glShaderSource(shader, 1, &src, NULL);

	glCompileShader(shader);

	// Error handling
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint length; 
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		if (length != 0)
		{
			char* message = malloc(length);
			glGetShaderInfoLog(shader, length, NULL, message);
			printf("%s", message);
			free(message);
		}
	}

	return shader;
}

GLuint BuildShader(const char* vertex_src, const char* fragment_src)
{
	GLuint vertex_shader = CompileShader(vertex_src, GL_VERTEX_SHADER);
	GLuint fragment_shader = CompileShader(fragment_src, GL_FRAGMENT_SHADER);

	GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	glLinkProgram(program);

	// Error handling
	glValidateProgram(program);
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		char* message = NULL;
		GLint length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
		if (length != 0)
		{
			message = malloc(length);
			glGetProgramInfoLog(program, length, NULL, message);
		}

		printf("Failed to link shader: %s", message);

		if (message)
		{
			free(message);
		}
	}

	// Cleanup
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	return program;
}

void DestroyShader(GLuint shader)
{
	glDeleteProgram(shader);
}