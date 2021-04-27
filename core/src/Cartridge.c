#include "Cartridge.h"
#include "Mappers/Mapper_000.h"
#include "Mappers/Mapper_001.h"
#include "Mappers/Mapper_002.h"
#include "Mappers/Mapper_003.h"
#include "Mappers/Mapper_004.h"
#include "Mappers/Mapper_JUN.h"
#include "nes.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// TODO: error handling for each mapper loading function

int load_cartridge_from_file(Nes* nes, const char* filepath, UPDATE_PATTERN_TABLE_CB callback)
{
	Cartridge* cart = &nes->cart;
	cart->updatePatternTableCB = callback;
	FILE* file = fopen(filepath, "rb");
	
	fread(&cart->header, sizeof(Header), 1, file);
	Header header = cart->header;

	// Verify File
	char* c = header.idstring;
	if (c[0] == 'N' && c[1] == 'E' && c[2] == 'S' && c[3] == 0x1A)
	{
		uint16_t mapperID = ((uint16_t)header.MapperID3 << 8) | ((uint16_t)header.MapperID2 << 4) | (uint16_t)header.MapperID1;
		cart->mapperID = mapperID;
		switch (mapperID)
		{
		case 0:
			m000LoadFromFile(&header, cart, file);
			break;
		case 1:
			m001LoadFromFile(&header, cart, file);
			break;
		case 2:
			m002LoadFromFile(&header, cart, file);
			break;
		case 3:
			m003LoadFromFile(&header, cart, file);
			break;
		case 4:
			m004LoadFromFile(&header, cart, file, &nes->cpu);
			break;
		default:
			printf("[ERROR] Not Yet implemented mapper id %i\n", mapperID);
			fclose(file);
			return 1;
		}
	}
	// My custom rom format, for now this format has a 16 byte header, then 64KB of data for cpu memory
	else if (c[0] == 'J' && c[1] == 'U' && c[2] == 'N' && c[3] == 0x1A)
	{
		mJUNLoadFromFile(cart, file);
	}
	else
	{
		printf("[ERROR] File %s unknown format\n", filepath);
		fclose(file);
		return 1;
	}

	fclose(file);
	return 0;
}

void free_cartridge(Cartridge* cart)
{
	switch (cart->mapperID)
	{
	case 0:
		m000Free(cart->mapper);
		break;
	case 1:
		m001Free(cart->mapper);
		break;
	case 2:
		m002Free(cart->mapper);
		break;
	case 3:
		m003Free(cart->mapper);
		break;
	case 4:
		m004Free(cart->mapper);
		break;
	case 767:
		free(cart->mapper);
		break;
	default:
		printf("[ERROR] Unknown mapper id, %i", cart->mapperID);
		break;
	}

	memset(cart, 0, sizeof(Cartridge));
}

int ines_file_format(Header* header)
{
	char* c = header->idstring;
	bool ines1 = c[0] == 'N' && c[1] == 'E' && c[2] == 'S' && c[3] == 0x1A;
	bool ines2 = ines1 && (header->FormatIdentifer == 0x2);
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

	if ((ines_file_format(header) == INES2_0) && (header->CHRRAM_Size.non_volatile_shift_count != 0 || header->CHRRAM_Size.volatile_shift_count != 0))
	{
		return true;
	}
	return false;
}