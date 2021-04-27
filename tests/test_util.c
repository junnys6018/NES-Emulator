#include "test_util.h"
#include <stdlib.h>
#include <stdio.h>

#include "string_util.h"
#include "Backend/nes.h"

void EmulateUntilHalt(Nes* nes, int instructions_per_frame)
{
	// Used to detect if cpu has halted
	uint16_t old_PC[32];

	int instructions_done = 0;

	while (true)
	{
		if (instructions_done < instructions_per_frame)
		{
			// Perform 5000 instructions at once
			for (int i = 0; i < 5000; i++)
			{
				clock_nes_instruction(nes);
				old_PC[i % 32] = nes->cpu.PC;
			}
			instructions_done += 5000;

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
}

int TestBlarggRom(const char* name, uint16_t result_addr)
{
	Nes nes;
	InitNES(&nes, name, NULL);

	EmulateUntilHalt(&nes, 100000);

	// Check for success or failure
	uint8_t result = cpu_bus_read(&nes.cpu_bus, result_addr);
	if (result == 1)
	{
		printf("Passed %s Test\n", GetFileName(name));
	}
	else
	{
		printf("Failed %s Test [%i]\n", GetFileName(name), result);
	}

	NESDestroy(&nes);

	return result == 1 ? 0 : 1;
}