#ifndef NAME_TABLE_MODEL_H
#define NAME_TABLE_MODEL_H
#include <stdint.h>
#include <glad/glad.h>
#include "../OpenGL/Texture.h"

// Textures representing the pattern tables currently accessible on the PPU
typedef struct
{
	Texture2D left_nametable;
	Texture2D right_nametable;
	uint8_t* left_nt_data;
	uint8_t* right_nt_data;
} NameTableModel;

#endif
