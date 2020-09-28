#include "Benchmarks.h"
#include "Backend/nes.h"

#include <timer.h>
#include <stdio.h>

void Run6502Benchmark()
{
	Nes nes;
	NESInit(&nes, "tests/roms/6502_functional_test.bin");

	int NUM = 100000000;
	timepoint beg, end;
	GetTime(&beg);
	for (int i = 0; i < NUM; i++)
	{
		clock_6502(&nes.cpu);
	}
	GetTime(&end);
	float time = GetElapsedTimeMicro(&beg, &end);
	float frequency_MHZ = (float)NUM / (time);

	printf("[6502 BENCHMARK] Max clock speed: %.5f MHz (1.789773 MHz Required)\n", frequency_MHZ);
}

void RunAllBenchmarks()
{
	Run6502Benchmark();
}
