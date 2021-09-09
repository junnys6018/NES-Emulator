#include "TestUtil.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Nes.h"

void emulate_until_halt(Nes* nes)
{
	// Used to detect if cpu has halted
	uint16_t old_PC[32];

	while (true)
	{
		// Perform 5000 instructions at once
		for (int i = 0; i < 5000; i++)
		{
			clock_nes_instruction(nes);
			old_PC[i % 32] = nes->cpu.PC;
		}

		bool halted = true;
		for (int i = 0; i < 32; i++)
		{
			halted = halted && (old_PC[i] == nes->cpu.PC);
		}
		if (halted)
		{
			break;
		}
	}
}

char* get_file_name(const char* filepath)
{
	return strrchr(filepath, '/') + 1;
}

int test_blargg_rom(const char* name, uint16_t result_addr)
{
	Nes nes;
	initialize_nes(&nes, name, NULL);

	emulate_until_halt(&nes);

	// Check for success or failure
	uint8_t result = cpu_bus_read(&nes.cpu_bus, result_addr);
	if (result == 1)
	{
		printf("Passed %s Test\n", get_file_name(name));
	}
	else
	{
		printf("Failed %s Test [%i]\n", get_file_name(name), result);
	}

	destroy_nes(&nes);

	return result == 1 ? 0 : 1;
}