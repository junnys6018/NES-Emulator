#include "BatchRenderer.h"
#include "Shader.h"
#include "../Renderer.h"

#include <glad/glad.h>
#include <stdbool.h>
#include <stdio.h>

#define QUADS_PER_BATCH 1024

typedef struct
{
	float x, y;
	float r, g, b;
	float tx, ty;
} Vertex;

static GLuint vbo, ibo, vao, shader;
static GLint u_transform;
static bool initialized = false;

static Vertex* buffer_ptr;
static unsigned int buffer_offset;

void InitBatchRenderer()
{
	if (!initialized)
	{
		initialized = true;

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, QUADS_PER_BATCH * sizeof(Vertex) * 4, NULL, GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2*sizeof(float)));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(5*sizeof(float)));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

		GLuint indices[QUADS_PER_BATCH * 6];
		for (int i = 0; i < QUADS_PER_BATCH; i++)
		{
			GLuint arr_offset = 6 * i;
			GLuint idx_offset = 4 * i;
			indices[arr_offset + 0] = idx_offset + 0;
			indices[arr_offset + 1] = idx_offset + 1;
			indices[arr_offset + 2] = idx_offset + 2;

			indices[arr_offset + 3] = idx_offset + 0;
			indices[arr_offset + 4] = idx_offset + 2;
			indices[arr_offset + 5] = idx_offset + 3;
		}

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		static const char* vertex_src =
			"#version 330 core\n"
			"layout(location = 0) in vec2 a_Position;\n"
			"layout(location = 1) in vec3 a_Color;\n"
			"layout(location = 2) in vec2 a_TexCoord;\n"
			"uniform mat4 u_transform;"
			"out vec3 v_Color;\n"
			"out vec2 v_TexCoord;\n"
			"void main() {\n"
			"    gl_Position = u_transform * vec4(a_Position, 0.0, 1.0);\n"
			"    v_Color = a_Color;\n"
			"    v_TexCoord = a_TexCoord;\n"
			"}\n";

		static const char* fragment_src =
			"#version 330 core\n"
			"in vec3 v_Color;\n"
			"in vec2 v_TexCoord;\n"
			"uniform sampler2D atlas;\n"
			"out vec4 color;\n"
			"void main() {\n"
			"    color = vec4(v_Color, 1.0);\n"
			"}\n";

		shader = BuildShader(vertex_src, fragment_src);

		u_transform = glGetUniformLocation(shader, "u_transform");
	}
	else
	{
		printf("Batch renderer already initialized!");
	}
}

void ShutdownBatchRenderer()
{
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
	glDeleteVertexArrays(1, &vao);
	DestroyShader(shader);
}

void BeginBatch()
{
	glBindVertexArray(vao);
	glUseProgram(shader);

	buffer_ptr = glMapNamedBuffer(vbo, GL_READ_WRITE);
	buffer_offset = 0;

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

	glUniformMatrix4fv(u_transform, 1, GL_FALSE, transform);
#undef T
}

void EndBatch()
{
	glUnmapNamedBuffer(vbo);
	glDrawElements(GL_TRIANGLES, buffer_offset * 6 / 4, GL_UNSIGNED_INT, 0);
}

void flush()
{
	glUnmapNamedBuffer(vbo);
	glDrawElements(GL_TRIANGLES, buffer_offset * 6 / 4, GL_UNSIGNED_INT, 0);
	buffer_ptr = glMapNamedBuffer(vbo, GL_READ_WRITE);
	buffer_offset = 0;

	printf("fushed");
}

void SubmitColoredQuad(int x, int y, int w, int h, int r, int g, int b)
{
	if (buffer_offset == QUADS_PER_BATCH * 4)
	{
		flush();
	}
	Vertex* v = buffer_ptr + buffer_offset;
	float _r = (float)r / 255.0f;
	float _g = (float)g / 255.0f;
	float _b = (float)b / 255.0f;

	v->x = x;
	v->y = y;

	v->r = _r;
	v->g = _g;
	v->b = _b;
	v++;

	v->x = x + w;
	v->y = y;

	v->r = _r;
	v->g = _g;
	v->b = _b;
	v++;

	v->x = x + w;
	v->y = y + h;

	v->r = _r;
	v->g = _g;
	v->b = _b;
	v++;

	v->x = x;
	v->y = y + h;

	v->r = _r;
	v->g = _g;
	v->b = _b;
	v++;

	buffer_offset += 4;
}

void SubmitTexturedQuad(int x, int y, int w, int h, int atlas_index)
{
}
