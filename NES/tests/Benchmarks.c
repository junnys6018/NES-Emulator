#include "Benchmarks.h"
#include "Backend/6502.h"

#include <time.h>
#include <stdio.h>

void Run6502Benchmark()
{
	Bus6502 bus;
	State6502 cpu;
	load_cpu_from_file(&cpu, &bus, "tests/6502_functional_test.bin");
	cpu.PC = 0x0400; // Code segment at 0x400

	int NUM = 100000000;
	time_t start = clock();
	for (int i = 0; i < NUM; i++)
	{
		clock_6502(&cpu);
	}
	time_t end = clock();
	float time_seconds = (float)(end - start) / CLOCKS_PER_SEC;
	float frequency_mHZ = (float)NUM / (time_seconds * 1000000);

	printf("[6502 BENCHMARK] Max clock speed: %.5f MHz (1.789773 MHz Required)\n", frequency_mHZ);
}

void RunAllBenchmarks()
{
	Run6502Benchmark();
}
