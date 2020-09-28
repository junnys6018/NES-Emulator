#include "Cartridge.h"
#include "Mappers/Mapper_000.h"
#include "Mappers/Mapper_JUN.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

void load_cartridge_from_file(Cartridge* cart, const char* filepath)
{
	FILE* file = fopen(filepath, "rb");
	struct
	{
		char idstring[4];
		uint8_t PRGROM_LSB;
		uint8_t CHRROM_LSB;

		// Flags
		uint8_t MirrorType : 1; // 0: Horizontal or mapper controlled; 1: vertical
		uint8_t Battery : 1;
		uint8_t Trainer : 1;
		uint8_t FourScreen : 1;
		uint8_t MapperID1 : 4; // Bits 0-3

		uint8_t ConsoleType : 2;
		uint8_t FormatIdentifer : 2; // Used to determine if the supplied file is a NES 1.0 or 2.0 format
		uint8_t MapperID2 : 4; // Bits 4-7

		uint8_t MapperID3 : 4; // Bits 8-11
		uint8_t SubmapperNumber : 4;

		uint8_t PRGROM_MSB : 4;
		uint8_t CHRROM_MSB : 4;

		uint8_t todo1; // TODO: Figure out what these bytes do
		uint8_t todo2;

		uint8_t TimingMode : 2;
		uint8_t unused : 6;

		uint8_t PPUType : 4;
		uint8_t HardwareType : 4;

		uint8_t MiscRomsPresent;
		uint8_t DefaultExpansionDevice;

	} header;
	fread(&header, sizeof(header), 1, file);

	// Verify File
	char* c = header.idstring;
	if (c[0] == 'N' && c[1] == 'E' && c[2] == 'S' && c[3] == 0x1A)
	{
		uint16_t mapperID = ((uint16_t)header.MapperID3 << 8) | ((uint16_t)header.MapperID2 << 4) | (uint16_t)header.MapperID1;
		cart->mapperID = mapperID;
		switch (mapperID)
		{
		case 0:
		{
			cart->CPUReadCartridge = m000CPUReadCartridge;
			cart->PPUReadCartridge = m000PPUReadCartridge;

			cart->CPUWriteCartridge = m000CPUWriteCartridge;
			cart->PPUWriteCartridge = m000PPUWriteCartridge;

			cart->PPUMirrorNametable = m000PPUMirrorNametable;

			Mapper000* map = malloc(sizeof(Mapper000));
			assert(map);
			cart->mapper = map;

			// Skip trainer if present
			if (header.Trainer)
			{ 
				fseek(file, 512, SEEK_CUR);
			}
			// Get the size of PRG ROM and read into cartridge
			uint16_t PRGROM_Size = ((uint16_t)header.PRGROM_MSB << 8) | (uint16_t)header.PRGROM_LSB;

			if (PRGROM_Size == 1)
			{
				map->romCap = _16K;
				map->PRG_ROM = malloc(16 * 1024);
				fread(map->PRG_ROM, 16 * 1024, 1, file);
			}
			else if (PRGROM_Size == 2)
			{
				map->romCap = _32K;
				map->PRG_ROM = malloc(32 * 1024);
				fread(map->PRG_ROM, 32 * 1024, 1, file);
			}
			else
			{
				printf("[ERROR] Invalid PRG ROM size\n");
			}

			// Read CHR ROM into cartridge
			fread(map->CHR, 8 * 1024, 1, file);

			// Set Nametable mirroring mode
			map->mirrorMode = header.MirrorType == 0 ? HORIZONTAL : VERTICAL;

			break;
		}
		default:
			printf("[ERROR] Not Yet implemented mapper id %i\n", mapperID);
			break;
		}
	}
	// My custom rom format, for now this format has a 16 byte header, then 64KB of data for cpu memory
	else if (c[0] == 'J' && c[1] == 'U' && c[2] == 'N' && c[3] == 0x1A)
	{
		cart->mapperID = 767; // Assigm mapperID 767 to my format
		
		cart->CPUReadCartridge = mJUNCPUReadCartridge;
		cart->PPUReadCartridge = mJUNPPUReadCartridge;

		cart->CPUWriteCartridge = mJUNCPUWriteCartridge;
		cart->PPUWriteCartridge = mJUNPPUWriteCartridge;

		cart->PPUMirrorNametable = mJUNPPUMirrorNametable;

		MapperJUN* map = malloc(sizeof(MapperJUN));
		assert(map);
		cart->mapper = map;

		fread(map->PRG_RAM, 64 * 1024, 1, file);
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
	case 767:
		free(cart->mapper);
		break;
	default:
		printf("[ERROR] Unknown mapper id, %i", cart->mapperID);
		break;
	}

	memset(cart, 0, sizeof(Cartridge));
}
