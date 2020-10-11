#include "Cartridge.h"
#include "Mappers/Mapper_000.h"
#include "Mappers/Mapper_001.h"
#include "Mappers/Mapper_JUN.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

void load_cartridge_from_file(Cartridge* cart, const char* filepath)
{
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
		default:
			printf("[ERROR] Not Yet implemented mapper id %i\n", mapperID);
			break;
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
	}


	fclose(file);
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
		return 1;
	}
	else if (ines1)
	{
		return 0;
	}
	return -1;
}
