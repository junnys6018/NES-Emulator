#include "Cartridge.h"
#include "Mappers/Mapper000.h"
#include "Mappers/Mapper001.h"
#include "Mappers/Mapper002.h"
#include "Mappers/Mapper003.h"
#include "Mappers/Mapper004.h"
#include "Mappers/MapperJUN.h"
#include "nes.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// TODO: error handling for each mapper loading function

int load_cartridge_from_file(struct Nes* nes, const char* filepath, UPDATE_PATTERN_TABLE_CB callback, char error_string[256])
{
	Cartridge* cart = &((Nes*)nes)->cart;

	cart->cartridge_file = malloc(strlen(filepath) + 1);
	if (!cart->cartridge_file)
	{
		if (error_string)
			sprintf(error_string, "%s", "memory allocation failed");

		return 1;
	}
	strcpy(cart->cartridge_file, filepath);

	cart->update_pattern_table_cb = callback;
	FILE* file = fopen(filepath, "rb");
	if (!file)
	{
		if (error_string)
			sprintf(error_string, "failed to open %s", filepath);
		return 1;
	}

	fread(&cart->header, sizeof(Header), 1, file);
	Header header = cart->header;

	// Verify File
	char* c = header.idstring;
	if (c[0] == 'N' && c[1] == 'E' && c[2] == 'S' && c[3] == 0x1A)
	{
		uint16_t mapper_id = ((uint16_t)header.mapper_id3 << 8) | ((uint16_t)header.mapper_id2 << 4) | (uint16_t)header.mapper_id1;
		cart->mapper_id = mapper_id;
		switch (mapper_id)
		{
		case 0:
			m000_load_from_file(&header, cart, file);
			break;
		case 1:
			m001_load_from_file(&header, cart, file);
			break;
		case 2:
			m002_load_from_file(&header, cart, file);
			break;
		case 3:
			m003_load_from_file(&header, cart, file);
			break;
		case 4:
			m004_load_from_file(&header, cart, file, &((Nes*)nes)->cpu);
			break;
		default:
			if (error_string)
				sprintf(error_string, "cannot load cartridge with mapper id %i", mapper_id);
			fclose(file);
			return 1;
		}
	}
	// My custom rom format, for now this format has a 16 byte header, then 64KB of data for cpu memory
	else if (c[0] == 'J' && c[1] == 'U' && c[2] == 'N' && c[3] == 0x1A)
	{
		mjun_load_from_file(cart, file);
	}
	else
	{
		if (error_string)
			sprintf(error_string, "invalid header");
		fclose(file);
		return 1;
	}

	fclose(file);

	// check if a save file exists in the same location as the rom
	char* save_location = get_default_save_location(filepath);
	struct stat buffer;
	if (stat(save_location, &buffer) == 0)
	{
		if (load_save(cart, save_location, NULL) == 0)
		{
			printf("[INFO]: loaded save file %s\n", save_location);
		}
	}

	free(save_location);
	return 0;
}

void free_cartridge(Cartridge* cart)
{
	free(cart->cartridge_file);
	switch (cart->mapper_id)
	{
	case 0:
		m000_free(cart->mapper);
		break;
	case 1:
		m001_free(cart->mapper);
		break;
	case 2:
		m002_free(cart->mapper);
		break;
	case 3:
		m003_free(cart->mapper);
		break;
	case 4:
		m004_free(cart->mapper);
		break;
	case 767:
		free(cart->mapper);
		break;
	default:
		printf("[ERROR] unknown mapper id %i\n", cart->mapper_id);
		break;
	}

	memset(cart, 0, sizeof(Cartridge));
}

int save_game(Cartridge* cart, const char* savefile, char error_string[256])
{
	switch (cart->mapper_id)
	{
	// mappers without save states are captured here
	case 0:
	case 2:
	case 3:
	case 767:
		return SAVE_NOT_SUPPORTED;
	case 1:
		return m001_save_game(cart, savefile, error_string);
	case 4:
		return m004_save_game(cart, savefile, error_string);
	default:
		if (error_string)
			sprintf(error_string, "unknown mapper id %i", cart->mapper_id);
		return 1;
	}
}

int load_save(Cartridge* cart, const char* savefile, char error_string[256])
{
	switch (cart->mapper_id)
	{
	// mappers without save states are captured here
	case 0:
	case 2:
	case 3:
	case 767:
		return 0;
	case 1:
		return m001_load_save(cart, savefile, error_string);
	case 4:
		return m004_load_save(cart, savefile, error_string);
	default:
		if (error_string)
			sprintf(error_string, "unknown mapper id %i", cart->mapper_id);
		return 1;
	}
}

// path/to/file.nes     -> path/to/file.sav
// path/to/file         -> path/to/file.sav
// path.dir/to/file.nes -> path.dir/to/file.sav
// path.dir/to/file     -> path.dir/to/file.sav
// file.nes             -> file.sav
// file                 -> file.sav
char* get_default_save_location(const char* rom_location)
{
	int len = strlen(rom_location);
	int idx = len - 1;
	while (idx >= 0 && rom_location[idx] != '/' && rom_location[idx] != '\\' && rom_location[idx] != '.')
		idx--;

	int count;
	if (rom_location[idx] == '/' || rom_location[idx] == '//' || idx == -1)
		count = len;
	else
		count = idx;

	char* save_location = malloc(count + 5); // 5 characters to append .sav\0
	strncpy(save_location, rom_location, count);
	strcpy(save_location + count, ".sav");

	return save_location;
}

int ines_file_format(Header* header)
{
	char* c = header->idstring;
	bool ines1 = c[0] == 'N' && c[1] == 'E' && c[2] == 'S' && c[3] == 0x1A;
	bool ines2 = ines1 && (header->format_identifier == 0x2);
	if (ines2)
	{
		return INES2_0;
	}
	else if (ines1)
	{
		return INES1_0;
	}
	return INES_UNKNOWN;
}

uint16_t num_prg_banks(Header* header)
{
	return ((uint16_t)header->PRGROM_MSB << 8) | (uint16_t)header->PRGROM_LSB;
}

uint16_t num_chr_banks(Header* header)
{
	return ((uint16_t)header->CHRROM_MSB << 8) | (uint16_t)header->CHRROM_LSB;
}

bool chr_is_ram(Header* header)
{
	if (ines_file_format(header) == INES1_0 && num_chr_banks(header) == 0)
	{
		return true;
	}

	if ((ines_file_format(header) == INES2_0) && (header->CHRRAM_size.non_volatile_shift_count != 0 || header->CHRRAM_size.volatile_shift_count != 0))
	{
		return true;
	}
	return false;
}