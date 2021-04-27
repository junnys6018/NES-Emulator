#include "Debug.h"
#include <glad/glad.h>
#include <stdio.h>

void OpenGLLogMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	const char* level;
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		level = "OpenGL Debug HIGH";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		level = "OpenGL Debug MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_LOW:
		level = "OpenGL Debug LOW";
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		return;
	}
	printf("[%s] %s\n", level, message);
}

void EnableGLDebugging()
{
	glDebugMessageCallback(OpenGLLogMessage, NULL);
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
}
