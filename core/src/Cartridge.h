#ifndef CARTRIDGE_H
#define CARTRIDGE_H
#include <stdint.h>
#include <stdbool.h>
struct Cartridge;
typedef uint8_t (*CPU_READ_BYTE)(struct Cartridge* cart, uint16_t addr, bool* read);
typedef void (*CPU_WRITE_BYTE)(struct Cartridge* cart, uint16_t addr, uint8_t data, bool* wrote);
typedef uint8_t (*PPU_READ_BYTE)(struct Cartridge* cart, uint16_t addr);
typedef void (*PPU_WRITE_BYTE)(struct Cartridge* cart, uint16_t addr, uint8_t data);
typedef void (*UPDATE_PATTERN_TABLE_CB)(uint8_t* table, int side);

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
	uint8_t mirror_type : 1; // 0: Horizontal or mapper controlled; 1: vertical
	uint8_t battery : 1;
	uint8_t trainer : 1;
	uint8_t four_screen : 1;
	uint8_t mapper_id1 : 4; // Bits 0-3

	uint8_t console_type : 2;
	uint8_t format_identifier : 2; // Used to determine if the supplied file is a NES 1.0 or 2.0 format, on a NES 1.0 it will have the value 0b00, on NES 2.0 it will read 0b10
	uint8_t mapper_id2 : 4; // Bits 4-7

	uint8_t mapper_id3 : 4; // Bits 8-11
	uint8_t submapper_number : 4;

	uint8_t PRGROM_MSB : 4;
	uint8_t CHRROM_MSB : 4;

	struct
	{
		uint8_t volatile_shift_count : 4;
		uint8_t non_volatile_shift_count : 4;
	} PRGRAM_size;

	struct
	{
		uint8_t volatile_shift_count : 4;
		uint8_t non_volatile_shift_count : 4;
	} CHRRAM_size;

	uint8_t timing_mode : 2;
	uint8_t unused : 6;

	uint8_t ppu_type : 4;
	uint8_t hardware_type : 4;

	uint8_t misc_roms_present;
	uint8_t default_expansion_device;

} Header;

typedef struct
{
	// For debugging
	Header header;

	uint16_t mapper_id;
	CPU_READ_BYTE cpu_read_cartridge;
	CPU_WRITE_BYTE cpu_write_cartridge;

	PPU_READ_BYTE ppu_read_cartridge;
	PPU_READ_BYTE ppu_peak_cartridge;
	PPU_WRITE_BYTE ppu_write_cartridge;

	NAMETABLE_MIRROR ppu_mirror_nametable;

	UPDATE_PATTERN_TABLE_CB update_pattern_table_cb;

	void* mapper;
} Cartridge;

// Returns 0 on success; non zero on failure
struct Nes;
int load_cartridge_from_file(struct Nes* nes, const char* filepath, UPDATE_PATTERN_TABLE_CB callback);
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

#endif
