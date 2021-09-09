#ifndef TEST_UTIL_H
#define TEST_UTIL_H
#include "nes.h"

void emulate_until_halt(Nes* nes);
int test_blargg_rom(const char* name, uint16_t result_addr); // Return 0 on success; 1 on failure
#endif
