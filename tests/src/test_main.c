#include "test_6502.h"
#include "test_2C02.h"
#include "Benchmarks.h"

int main(int argc, char** argv)
{
	run_all_6502_tests();
	run_all_2C02_tests();
	run_all_benchmarks();
}