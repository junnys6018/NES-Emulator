#include "Scanline.h"
#include "Shader.h"
#include "Framebuffer.h"
#include "../Controller.h"
#include <glad/glad.h>

static GLuint vbo, vao, shader;
static Framebuffer framebuffer;
static GLint u_time;
void InitScanlineEffect()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	float quadVertices[] = {
		// positions     // texCoords
		-1.0f,  1.0f,	  0.0f, 1.0f,
		-1.0f, -1.0f,	  0.0f, 0.0f,
		 1.0f, -1.0f,	  1.0f, 0.0f,

		-1.0f,  1.0f,	  0.0f, 1.0f,
		 1.0f, -1.0f,	  1.0f, 0.0f,
		 1.0f,  1.0f,	  1.0f, 1.0f
	};

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2*sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	static const char* vertex_src =
		"#version 330 core\n"
		"layout(location = 0) in vec2 a_Position;\n"
		"layout(location = 1) in vec2 a_TexCoord;\n"
		"out vec2 v_TexCoord;\n"
		"void main() {\n"
		"    gl_Position = vec4(a_Position, 0.0, 1.0);\n"
		"    v_TexCoord = a_TexCoord;\n"
		"}\n";

	static const char* fragment_src =
		"#version 330 core\n"
		"in vec2 v_TexCoord;\n"
		"uniform sampler2D u_source;\n"
		"uniform float u_time;\n"
		"out vec4 color;\n"
		"void main() {\n"
		"	// vec3 sample = texture(u_source, v_TexCoord).rgb;\n"
		"	vec3 sample;\n"
		"\n"
		"	sample.r = texture(u_source, vec2(v_TexCoord.x - 0.007 * sin(u_time), v_TexCoord.y)).r;\n"
		"	sample.g = texture(u_source, v_TexCoord).g;\n"
		"	sample.b = texture(u_source, vec2(v_TexCoord.x + 0.007 * sin(u_time), v_TexCoord.y)).b;\n"
		"\n"
		"	sample -= vec3(sin(gl_FragCoord.y * 2.0 + u_time * 16.0) * 0.02);\n"
		"   color = vec4(sample, 1.0);\n"
		"}\n";

	shader = BuildShader(vertex_src, fragment_src);
	glUseProgram(shader);
	glUniform1i(glGetUniformLocation(shader, "u_source"), 0);
	u_time = glGetUniformLocation(shader, "u_time");

	GenerateFramebuffer(&framebuffer, 256, 240);
}

void ShutdownScanlineEffect()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	DestroyShader(shader);
	DeleteFramebuffer(&framebuffer);
}

GLuint ScanlineOnDraw(float time, GLuint source)
{
	glBindVertexArray(vao);
	glUseProgram(shader);
	glBindTextureUnit(0, source);
	glUniform1f(u_time, time);

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
	glViewport(0, 0, 256, 240);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	int w, h;
	GetWindowSize(&w, &h);
	glViewport(0, 0, w, h);

	return framebuffer.texture.handle;
}
