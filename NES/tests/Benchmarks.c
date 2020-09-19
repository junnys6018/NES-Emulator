#include "Benchmarks.h"
#include "Backend/6502.h"

#include "timer.h"
#include <stdio.h>

void Run6502Benchmark()
{
	Bus6502 bus;
	State6502 cpu;
	load_cpu_from_file(&cpu, &bus, "tests/6502_functional_test.bin");
	cpu.PC = 0x0400; // Code segment at 0x400

	int NUM = 100000000;
	timepoint beg, end;
	GetTime(&beg);
	for (int i = 0; i < NUM; i++)
	{
		clock_6502(&cpu);
	}
	GetTime(&end);
	float time_micro = GetElapsedTimeMicro(&beg, &end);
	float frequency_mHZ = (float)NUM / time_micro;

	printf("[6502 BENCHMARK] Max clock speed: %.5f MHz (1.789773 MHz Required)\n", frequency_mHZ);
}

void RunAllBenchmarks()
{
	Run6502Benchmark();
}
