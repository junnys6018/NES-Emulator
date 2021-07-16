#include "BatchRenderer.h"
#include "Shader.h"
#include "Texture.h"
#include "Application.h"

#include <glad/glad.h>
#include <stdbool.h>
#include <stdio.h>

#define QUADS_PER_BATCH 2048

typedef struct
{
	float x, y;
	float r, g, b;
	float tx, ty;
	float texture_slot;
} Vertex;

static GLuint vbo, ibo, vao, shader;
static Texture2D white_texture;
static GLint u_transform;
static bool initialized = false;

static Vertex* buffer_ptr;
static unsigned int buffer_offset;
static GLuint active_textures[31];
static int active_texture_index;

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
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2 * sizeof(float)));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(5 * sizeof(float)));
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(7 * sizeof(float)));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);

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
			"layout(location = 3) in float a_TexIndex;\n"
			"uniform mat4 u_transform;\n"
			"out vec3 v_Color;\n"
			"out vec2 v_TexCoord;\n"
			"out float v_TexIndex;\n"
			"void main() {\n"
			"    gl_Position = u_transform * vec4(a_Position, 0.0, 1.0);\n"
			"    v_Color = a_Color;\n"
			"    v_TexCoord = a_TexCoord;\n"
			"    v_TexIndex = a_TexIndex;\n"
			"}\n";

		static const char* fragment_src =
			"#version 330 core\n"
			"in vec3 v_Color;\n"
			"in vec2 v_TexCoord;\n"
			"in float v_TexIndex;\n"
			"uniform sampler2D textures[32];\n"
			"out vec4 color;\n"
			"void main() {\n"
			"	int index = int(v_TexIndex);"
			"   color = vec4(v_Color, 1.0) * texture(textures[index], v_TexCoord);\n"
			"}\n";

		shader = BuildShader(vertex_src, fragment_src);

		u_transform = glGetUniformLocation(shader, "u_transform");
		GLint texture_slots[32];
		for (int i = 0; i < 32; i++)
		{
			texture_slots[i] = i;
		}
		glUseProgram(shader);
		glUniform1iv(glGetUniformLocation(shader, "textures"), 32, texture_slots);

		// Create a white 1x1 texture
		uint32_t data = 0xFFFFFFFF;
		GenerateTexture(&white_texture, 1, 1, &data, GL_NEAREST, GL_RGB);
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
	DeleteTexture(&white_texture);
	DestroyShader(shader);
}

void BeginBatch()
{
	buffer_ptr = glMapNamedBuffer(vbo, GL_READ_WRITE);
	buffer_offset = 0;
	active_texture_index = 0;
}

void EndBatch()
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
	glBindTextureUnit(0, white_texture.handle);
	glBindVertexArray(vao);
	glUnmapNamedBuffer(vbo);

	glDrawElements(GL_TRIANGLES, buffer_offset * 6 / 4, GL_UNSIGNED_INT, 0);

	buffer_offset = 0;
	active_texture_index = 0;
}

void FlushQuads()
{
	EndBatch();
	buffer_ptr = glMapNamedBuffer(vbo, GL_READ_WRITE);
}

void SubmitTexturedColoredQuad(SDL_Rect* span, GLuint texture, float tx, float ty, float tw, float th, uint8_t r, uint8_t g, uint8_t b)
{
	int bound_index = 0;
	if (texture == 0)
	{
		bound_index = -1;
	}
	else
	{
		bool texture_already_bound = false;
		for (; bound_index < active_texture_index; bound_index++)
		{
			if (active_textures[bound_index] == texture)
			{
				texture_already_bound = true;
				break;
			}
		}

		if (!texture_already_bound)
		{
			bound_index = active_texture_index++;
			active_textures[bound_index] = texture;
			glBindTextureUnit(bound_index + 1, texture);
		}
	}

	if (tw == 0.0f || th == 0.0f)
	{
		tx = 0.0f;
		ty = 0.0f;
		tw = 1.0f;
		th = 1.0f;
	}

	Vertex* v = buffer_ptr + buffer_offset;
	float slot = bound_index + 1;
	float _r = (float)r / 255.0f;
	float _g = (float)g / 255.0f;
	float _b = (float)b / 255.0f;

	v->x = span->x;
	v->y = span->y;
	v->tx = tx;
	v->ty = ty;
	v->r = _r;
	v->g = _g;
	v->b = _b;
	v->texture_slot = slot;
	v++;

	v->x = span->x + span->w;
	v->y = span->y;
	v->tx = tx + tw;
	v->ty = ty;
	v->r = _r;
	v->g = _g;
	v->b = _b;
	v->texture_slot = slot;
	v++;

	v->x = span->x + span->w;
	v->y = span->y + span->h;
	v->tx = tx + tw;
	v->ty = ty + th;
	v->r = _r;
	v->g = _g;
	v->b = _b;
	v->texture_slot = slot;
	v++;

	v->x = span->x;
	v->y = span->y + span->h;
	v->tx = tx;
	v->ty = ty + th;
	v->r = _r;
	v->g = _g;
	v->b = _b;
	v->texture_slot = slot;

	buffer_offset += 4;
	if (buffer_offset == QUADS_PER_BATCH * 4 || active_texture_index == 31)
	{
		FlushQuads();
	}
}
