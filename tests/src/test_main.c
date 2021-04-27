#include "test_6502.h"
#include "test_2C02.h"
#include "Benchmarks.h"

int main(int argc, char** argv)
{
	RunAll6502Tests();
	RunAll2C02Tests();
	RunAllBenchmarks();
}