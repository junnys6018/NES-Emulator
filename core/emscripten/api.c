#include "emscripten.h"
#include "nes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EVENT_NONE 0
#define EVENT_NEW_FRAME 1
#define EVENT_AUDIO_BUFFER_FULL 2
#define EVENT_UNTIL_TICKS 4

EMSCRIPTEN_KEEPALIVE
void initialize()
{
	printf("[INFO] NES WebAssembly Instance Started\n");
	precompute_audio();
}

EMSCRIPTEN_KEEPALIVE
Nes* create_nes(uint8_t* rom, size_t romsize, uint8_t* save, size_t savesize)
{
	Nes* nes = malloc(sizeof(Nes));
	memset(nes, 0, sizeof(Nes));
	nes->cpu_bus.cartridge = &nes->cart;
	nes->cpu_bus.ppu = &nes->ppu;
	nes->cpu_bus.cpu = &nes->cpu;
	nes->cpu_bus.apu = &nes->apu;
	nes->cpu_bus.pad = &nes->pad;

	nes->ppu_bus.cartridge = &nes->cart;

	nes->cpu.bus = &nes->cpu_bus;

	nes->ppu.bus = &nes->ppu_bus;
	nes->ppu.cpu = &nes->cpu;

	nes->apu.cpu = &nes->cpu;

	FILE* romfile = fmemopen(rom, romsize, "rb");
	FILE* savefile = NULL;

	if (save)
		fmemopen(save, savesize, "rb");

	load_cartridge_from_file(nes, romfile, savefile, NULL, NULL);
	fclose(romfile);
	if (savefile)
		fclose(savefile);

	reset_nes(nes);

	return nes;
}

EMSCRIPTEN_KEEPALIVE
void free_nes(Nes* nes)
{
	destroy_nes(nes);
	free(nes);
}

EMSCRIPTEN_KEEPALIVE
void* create_buffer(size_t size)
{
	return malloc(size);
}

EMSCRIPTEN_KEEPALIVE
void free_buffer(void* buf)
{
	free(buf);
}

inline bool frame_ready(Nes* nes)
{
	return nes->ppu.scanline == 242 && nes->ppu.cycles == 0;
}

inline bool audio_ready(Nes* nes)
{
	return nes->apu.audio_pos >= 512;
}

EMSCRIPTEN_KEEPALIVE
uint32_t emulate_until(Nes* nes, uint64_t ppu_cycle)
{
	do
	{
		clock_nes_cycle(nes);
	} while (!frame_ready(nes) && !audio_ready(nes) && nes->system_clock < ppu_cycle);

	uint32_t event = EVENT_NONE;
	if (frame_ready(nes))
	{
		event |= EVENT_NEW_FRAME;
	}
	if (audio_ready(nes))
	{
		event |= EVENT_AUDIO_BUFFER_FULL;
	}
	if (nes->system_clock == ppu_cycle)
	{
		event |= EVENT_UNTIL_TICKS;
	}

	return event;
}

EMSCRIPTEN_KEEPALIVE
uint64_t get_total_cycles(Nes* nes)
{
	return nes->system_clock;
}

EMSCRIPTEN_KEEPALIVE
uint32_t* get_framebuffer_wasm(Nes* nes)
{
	return get_framebuffer(&nes->ppu);
}

EMSCRIPTEN_KEEPALIVE
void set_keys(Nes* nes, uint8_t _keys)
{
	Keys keys;
	keys.reg = _keys;
	poll_keys(&nes->pad, keys);
}

float audio_buffer[4096];

EMSCRIPTEN_KEEPALIVE
float* get_audio_buffer()
{
	return &audio_buffer[0];
}

EMSCRIPTEN_KEEPALIVE
uint32_t flush_audio_samples(Nes* nes)
{
	uint32_t ret = nes->apu.audio_pos;
	memcpy(audio_buffer, nes->apu.audio_buffer, ret * sizeof(float));
	nes->apu.audio_pos = 0;
	return ret;
}
