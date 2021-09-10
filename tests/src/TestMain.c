#include <stdio.h>
#include "Test6502.h"
#include "Test2C02.h"
#include "Benchmarks.h"

#if !defined(PLATFORM_WINDOWS) && !defined(PLATFORM_LINUX)
#error Unknown platform
#endif

#if !defined(CONFIGURATION_DEBUG) && !defined(CONFIGURATION_RELEASE)
#error Unknown configuration
#endif

#if defined(PLATFORM_WINDOWS)
static const char* platform_string = "Windows";
#elif defined(PLATFORM_LINUX)
static const char* platform_string = "Linux";
#endif

#if defined(CONFIGURATION_DEBUG)
static const char* configuration_string = "Debug";
#elif defined(CONFIGURATION_RELEASE)
static const char* configuration_string = "Release";
#endif

int main(int argc, char** argv)
{
	printf("Running NES tests (%s, %s)\n", platform_string, configuration_string);

	int num_failed = run_all_6502_tests();
	num_failed += run_all_2C02_tests();
	run_all_benchmarks();

	return num_failed > 0 ? 1 : 0;
}