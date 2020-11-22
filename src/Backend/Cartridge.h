#ifndef CARTRIDGE_H
#define CARTRIDGE_H
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t(*CPU_READ_BYTE)(void* mapper, uint16_t addr, bool* read);
typedef void(*CPU_WRITE_BYTE)(void* mapper, uint16_t addr, uint8_t data, bool* wrote);
typedef uint8_t(*PPU_READ_BYTE)(void* mapper, uint16_t addr);
typedef void(*PPU_WRITE_BYTE)(void* mapper, uint16_t addr, uint8_t data);

typedef struct
{
	uint16_t index : 1;
	uint16_t addr : 10;
} NametableIndex;
typedef NametableIndex(*NAMETABLE_MIRROR)(void* mapper, uint16_t addr);

typedef struct
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
	uint8_t FormatIdentifer : 2; // Used to determine if the supplied file is a NES 1.0 or 2.0 format, on a NES 1.0 it will have the value 0b00, on NES 2.0 it will read 0b10
	uint8_t MapperID2 : 4; // Bits 4-7

	uint8_t MapperID3 : 4; // Bits 8-11
	uint8_t SubmapperNumber : 4;

	uint8_t PRGROM_MSB : 4;
	uint8_t CHRROM_MSB : 4;

	struct
	{
		uint8_t volatile_shift_count : 4;
		uint8_t non_volatile_shift_count : 4;
	} PRGRAM_Size;

	struct
	{
		uint8_t volatile_shift_count : 4;
		uint8_t non_volatile_shift_count : 4;
	} CHRRAM_Size;

	uint8_t TimingMode : 2;
	uint8_t unused : 6;

	uint8_t PPUType : 4;
	uint8_t HardwareType : 4;

	uint8_t MiscRomsPresent;
	uint8_t DefaultExpansionDevice;

} Header;

typedef struct
{
	// For debugging
	Header header;

	uint16_t mapperID;
	CPU_READ_BYTE CPUReadCartridge;
	CPU_WRITE_BYTE CPUWriteCartridge;

	PPU_READ_BYTE PPUReadCartridge;
	PPU_READ_BYTE PPUPeakCartridge;
	PPU_WRITE_BYTE PPUWriteCartridge;

	NAMETABLE_MIRROR PPUMirrorNametable;

	void* mapper;
} Cartridge;

// Returns 0 on success; non zero on failure
int load_cartridge_from_file(struct Nes* nes, const char* filepath);
void free_cartridge(Cartridge* cart);


// Header utility functions

// 0: INES 1.0; 1: NES 2.0; -1: unknown
enum
{
	INES1_0 = 0,
	INES2_0 = 1,
	INES_UNKNOWN = -1,
};
int ines_file_format(Header* header);
uint16_t num_prg_banks(Header* header);
uint16_t num_chr_banks(Header* header);
bool chr_is_ram(Header* header);

#endif // !CARTRIDGE_H
