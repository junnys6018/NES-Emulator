#include "Benchmarks.h"
#include "nes.h"

#include <timer.h>
#include <stdio.h>

void run_6502_benchmark()
{
	Nes nes;
	initialize_nes(&nes, "roms/6502_functional_test.bin", NULL, NULL);

	int NUM = 100000000;
	timepoint beg, end;
	get_time(&beg);
	for (int i = 0; i < NUM; i++)
	{
		clock_6502(&nes.cpu);
	}
	get_time(&end);
	float time = get_elapsed_time_micro(&beg, &end);
	float frequency_MHZ = (float)NUM / time;

	printf("[6502 BENCHMARK] Max clock speed: %.5f MHz (1.789773 MHz Required)\n", frequency_MHZ);

	destroy_nes(&nes);
}

void run_2C02_benchmark()
{
	Nes nes;
	initialize_nes(&nes, "roms/palette.nes", NULL, NULL);

	nes.ppu.PPUMASK.reg = 0x18; // enable all rendering

	int NUM = 100000000;
	timepoint beg, end;
	get_time(&beg);
	for (int i = 0; i < NUM; i++)
	{
		clock_2C02(&nes.ppu);
	}
	get_time(&end);
	float time = get_elapsed_time_micro(&beg, &end);
	float frequency_MHZ = (float)NUM / time;

	printf("[2C02 BENCHMARK] Max clock speed: %.5f MHz (5.36931 MHz Required)\n", frequency_MHZ);

	destroy_nes(&nes);
}

void run_nes_benchmark()
{
	Nes nes;
	initialize_nes(&nes, "roms/SuperMarioBros.nes", NULL, NULL);

	int NUM = 100000000;
	timepoint beg, end;
	get_time(&beg);
	for (int i = 0; i < NUM; i++)
	{
		clock_nes_cycle(&nes);
	}
	get_time(&end);
	float time = get_elapsed_time_micro(&beg, &end);
	float frequency_MHZ = (float)NUM / time;

	printf("[NES BENCHMARK] Max clock speed: %.5f MHz (5.36931 MHz Required)\n", frequency_MHZ);


	destroy_nes(&nes);
}


void run_all_benchmarks()
{
	run_6502_benchmark();
	run_2C02_benchmark();
	run_nes_benchmark();
}
