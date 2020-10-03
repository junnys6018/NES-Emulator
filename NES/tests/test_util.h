#ifndef TEST_UTIL_H
#define TEST_UTIL_H
#include "Backend/nes.h"
#include <SDL.h>

Uint32 on_render_callback(Uint32 interval, void* param);

void EmulateUntilHalt(Nes* nes, int instructions_per_frame);
void TestBlarggRom(const char* name, uint16_t result_addr);
#endif // !TEST_UTIL_H
