#include "LineRenderer.h"
#include "Shader.h"
#include "Application.h"
#include <glad/glad.h>
#include <stdbool.h>

#define LINES_PER_BATCH 4096

typedef struct
{
	float x, y;
	float r, g, b;
} Vertex;

static GLuint vbo, vao, shader;
static GLint u_transform;
static bool initialized = false;

static Vertex* buffer_ptr;
static unsigned int buffer_offset;

void InitLineRenderer()
{
	if (!initialized)
	{
		initialized = true;

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, LINES_PER_BATCH * sizeof(Vertex) * 2, NULL, GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2 * sizeof(float)));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		static const char* vertex_src =
			"#version 330 core\n"
			"layout(location = 0) in vec2 a_Position;\n"
			"layout(location = 1) in vec3 a_Color;\n"
			"uniform mat4 u_transform;\n"
			"out vec3 v_Color;\n"
			"void main() {\n"
			"    gl_Position = u_transform * vec4(a_Position, 0.0, 1.0);\n"
			"    v_Color = a_Color;\n"
			"}\n";

		static const char* fragment_src =
			"#version 330 core\n"
			"in vec3 v_Color;\n"
			"out vec4 color;\n"
			"void main() {\n"
			"   color = vec4(v_Color, 1.0);\n"
			"}\n";

		shader = BuildShader(vertex_src, fragment_src);

		u_transform = glGetUniformLocation(shader, "u_transform");
	}
}

void ShutdownLineRenderer()
{
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	DestroyShader(shader);
}

void BeginLines()
{
	buffer_ptr = glMapNamedBuffer(vbo, GL_READ_WRITE);
	buffer_offset = 0;
}

void EndLines()
{
	// Set the transform matrix, transforms SDL screen coordinates into openGL NDC
	int w, h;
	GetWindowSize(&w, &h);

#define T(a, b) (b * 4 + a)
	float transform[16] = {0};
	transform[T(0, 0)] = 2.0f / w;
	transform[T(1, 1)] = -2.0f / h;
	transform[T(0, 3)] = -1.0f;
	transform[T(1, 3)] = 1.0f;
	transform[T(3, 3)] = 1.0f;
#undef T

	glUseProgram(shader);
	glUniformMatrix4fv(u_transform, 1, GL_FALSE, transform);
	glBindVertexArray(vao);
	glUnmapNamedBuffer(vbo);

	glDrawArrays(GL_LINES, 0, buffer_offset);

	buffer_offset = 0;
}

void FlushLines()
{
	EndLines();
	buffer_ptr = glMapNamedBuffer(vbo, GL_READ_WRITE);
}

void SubmitLine(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b)
{
	Vertex* v = buffer_ptr + buffer_offset;
	float _r = (float)r / 255.0f;
	float _g = (float)g / 255.0f;
	float _b = (float)b / 255.0f;

	v->x = x1;
	v->y = y1;
	v->r = _r;
	v->g = _g;
	v->b = _b;
	v++;

	v->x = x2;
	v->y = y2;
	v->r = _r;
	v->g = _g;
	v->b = _b;

	buffer_offset += 2;
	if (buffer_offset == LINES_PER_BATCH * 2)
	{
		FlushLines();
	}
}
