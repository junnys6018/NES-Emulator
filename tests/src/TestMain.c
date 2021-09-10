#include "Test6502.h"
#include "Test2C02.h"
#include "Benchmarks.h"

int main(int argc, char** argv)
{
	int num_failed = run_all_6502_tests();
	num_failed += run_all_2C02_tests();
	run_all_benchmarks();
	return num_failed > 0 ? 0 : 1;
}