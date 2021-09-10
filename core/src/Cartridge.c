#include "Cartridge.h"
#include "Mappers/Mapper000.h"
#include "Mappers/Mapper001.h"
#include "Mappers/Mapper002.h"
#include "Mappers/Mapper003.h"
#include "Mappers/Mapper004.h"
#include "Mappers/MapperJUN.h"
#include "Nes.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// TODO: error handling for each mapper loading function

int load_cartridge_from_file(struct Nes* nes, const char* filepath, UPDATE_PATTERN_TABLE_CB callback, char error_string[256])
{
	Cartridge* cart = &((Nes*)nes)->cart;
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
	return 0;
}

void free_cartridge(Cartridge* cart)
{
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
		printf("[ERROR] Unknown mapper id, %i", cart->mapper_id);
		break;
	}

	memset(cart, 0, sizeof(Cartridge));
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