#ifndef TEST_UTIL_H
#define TEST_UTIL_H
#include "Backend/nes.h"
#include <SDL.h>

Uint32 on_render_callback(Uint32 interval, void* param);

void EmulateUntilHalt(Nes* nes, int instructions_per_frame);
#endif // !TEST_UTIL_H
