#include "test_6502.h"

#include "Frontend/Renderer.h"
#include "Backend/nes.h"
#include "event_filter_function.h"

#include "timer.h"
#include "test_util.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <SDL.h>

int Run_6502_Functional_Test()
{
	Nes nes;
	NESInit(&nes, "tests/roms/6502_functional_test.bin");

	RendererBindNES(&nes);

	EmulateUntilHalt(&nes, 200000);

	// Check for success or failure
	uint8_t success_opcode_pattern[] = { 0x4C, 0xBA, 0xEA, 0xBA, 0xEA };
	uint8_t opcode_size[] = { 3, 1, 1, 1, 1 };
	bool passed = true;
	uint16_t pc = nes.cpu.PC;
	for (int i = 0; i < 5; i++)
	{
		uint8_t opcode = cpu_bus_read(&nes.cpu_bus, pc);
		passed = passed && (opcode == success_opcode_pattern[i]);
		pc += opcode_size[i];
	}
	if (passed)
		printf("Passed Functional Test\n");
	else
		printf("Failed Functional Test\n");

	RendererDraw();
	
	NESDestroy(&nes);

	return passed ? 0 : 1;
}

int Run_6502_Interrupt_Test()
{
	Nes nes;
	NESInit(&nes, "tests/roms/6502_interrupt_test.bin");

	RendererBindNES(&nes);
	RendererDraw();

	// Update screen every 16ms
	SDL_TimerID tid = SDL_AddTimer(16, on_render_callback, NULL);

	// Filter away events that are not used
	EventTypeList list = { .size = 2,.event_types = {SDL_USEREVENT, SDL_QUIT} };
	SDL_SetEventFilter(event_whitelist, &list);


	const uint16_t feedback_register_addr = 0xBFFC; // Location of register used to feed interrupts into the cpu
	uint8_t old_nmi = 0; // Used to detect a level change
	uint16_t old_PC = 0x400; // Used to detect if cpu has halted
	const int instructions_per_frame = 10; // Execution speed
	int instructions_done = 0;

	bool passed = true;
	SDL_Event event;
	while (true)
	{
		if (instructions_done < instructions_per_frame)
		{
			instructions_done++;

			while (clock_6502(&nes.cpu) != 0);
			uint8_t I_src = cpu_bus_read(&nes.cpu_bus, feedback_register_addr);
			if (I_src & 0x02 && old_nmi == 0) // NMI - Detected on rising edge
			{
				NMI(&nes.cpu);
			}
			if (I_src & 0x01) // IRQ - level detected
			{
				IRQ_Set(&nes.cpu, 7);
			}
			else
			{
				IRQ_Clear(&nes.cpu, 7);
			}

			old_nmi = I_src & 0x02; // Mask out NMI bit

			// Check for success or failure
			if (old_PC == nes.cpu.PC) // Execution paused in infinite loop
			{
				uint8_t success_opcode_pattern[] = { 0x4C, 0xBA, 0xEA, 0xBA, 0xEA };
				uint8_t opcode_size[] = { 3, 1, 1, 1, 1 };
				for (int i = 0; i < 5; i++)
				{
					uint8_t opcode = cpu_bus_read(&nes.cpu_bus, old_PC);
					passed = passed && (opcode == success_opcode_pattern[i]);
					old_PC += opcode_size[i];
				}

				if (passed)
					printf("Passed Interrupt Test\n");
				else
					printf("Failed Interrupt Test\n");

				RendererDraw();
				break;
			}

			old_PC = nes.cpu.PC;
		}


		// Poll Events
		while (SDL_PollEvent(&event) != 0)
		{
			if (event.type == SDL_QUIT)
			{
				exit(EXIT_SUCCESS);
			}
			else if (event.type == SDL_USEREVENT && event.user.code == 0)
			{
				RendererDraw();
				instructions_done = 0;
			}
		}

	}

	NESDestroy(&nes);

	SDL_RemoveTimer(tid);
	SDL_SetEventFilter(reset_filter_event, NULL);

	return passed ? 0 : 1;
}


void RunAll6502Tests()
{
	const int num_tests = 5;
	int num_failed = 0;
	num_failed += Run_6502_Interrupt_Test();
	num_failed += Run_6502_Functional_Test();

	num_failed += TestBlarggRom("tests/roms/blargg_tests/branch_timing_tests/1.Branch_Basics.nes", 0xF8);
	num_failed += TestBlarggRom("tests/roms/blargg_tests/branch_timing_tests/2.Backward_Branch.nes", 0xF8);
	num_failed += TestBlarggRom("tests/roms/blargg_tests/branch_timing_tests/3.Forward_Branch.nes", 0xF8);

	printf("[6502 TESTS] Passed: %i; Failed %i\n", num_tests - num_failed, num_failed);
}
