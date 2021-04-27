#ifndef TEST_UTIL_H
#define TEST_UTIL_H
#include "nes.h"

void EmulateUntilHalt(Nes* nes);
int TestBlarggRom(const char* name, uint16_t result_addr); // Return 0 on success; 1 on failure
#endif // !TEST_UTIL_H
