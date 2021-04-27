#include "Benchmarks.h"
#include "Backend/nes.h"

#include <timer.h>
#include <stdio.h>

void Run6502Benchmark()
{
	Nes nes;
	InitNES(&nes, "tests/roms/6502_functional_test.bin", NULL);

	int NUM = 100000000;
	timepoint beg, end;
	GetTime(&beg);
	for (int i = 0; i < NUM; i++)
	{
		clock_6502(&nes.cpu);
	}
	GetTime(&end);
	float time = GetElapsedTimeMicro(&beg, &end);
	float frequency_MHZ = (float)NUM / time;

	printf("[6502 BENCHMARK] Max clock speed: %.5f MHz (1.789773 MHz Required)\n", frequency_MHZ);

	NESDestroy(&nes);
}

void Run2C02Benchmark()
{
	Nes nes;
	InitNES(&nes, "tests/roms/palette.nes", NULL);

	nes.ppu.PPUMASK.reg = 0x18; // enable all rendering

	int NUM = 100000000;
	timepoint beg, end;
	GetTime(&beg);
	for (int i = 0; i < NUM; i++)
	{
		clock_2C02(&nes.ppu);
	}
	GetTime(&end);
	float time = GetElapsedTimeMicro(&beg, &end);
	float frequency_MHZ = (float)NUM / time;

	printf("[2C02 BENCHMARK] Max clock speed: %.5f MHz (5.36931 MHz Required)\n", frequency_MHZ);

	NESDestroy(&nes);
}

void RunNESBenchmark()
{
	Nes nes;
	InitNES(&nes, "tests/roms/SuperMarioBros.nes", NULL);

	int NUM = 100000000;
	timepoint beg, end;
	GetTime(&beg);
	for (int i = 0; i < NUM; i++)
	{
		clock_nes_cycle(&nes);
	}
	GetTime(&end);
	float time = GetElapsedTimeMicro(&beg, &end);
	float frequency_MHZ = (float)NUM / time;

	printf("[NES BENCHMARK] Max clock speed: %.5f MHz (5.36931 MHz Required)\n", frequency_MHZ);


	NESDestroy(&nes);
}


void RunAllBenchmarks()
{
	Run6502Benchmark();
	Run2C02Benchmark();
	RunNESBenchmark();
}
