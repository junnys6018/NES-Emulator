#include "emscripten.h"
#include "nes.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef enum
{
	EVENT_NONE = 0,
	EVENT_NEW_FRAME = 1,
	EVENT_AUDIO_BUFFER_FULL = 2,
	EVENT_UNTIL_TICKS = 4
} EmulatorEvent;

Nes nes;
bool first_load = true;

EMSCRIPTEN_KEEPALIVE
void LoadRom(char* rom)
{
	if (!first_load)
	{
		NESDestroy(&nes);
	}

	if (first_load)
	{
		AudioPrecompute();
		first_load = false;
	}
	
	InitNES(&nes, rom, NULL);
}

inline bool FrameReady()
{
	return nes.ppu.scanline == 242 && nes.ppu.cycles == 0;
}

inline bool AudioReady()
{
	return nes.apu.audio_pos >= 512;
}

EMSCRIPTEN_KEEPALIVE
EmulatorEvent EmulateUntil(size_t ppu_cycle)
{
	do 
	{
		clock_nes_cycle(&nes);
	} while (!FrameReady() && !AudioReady() && nes.system_clock < ppu_cycle);

	EmulatorEvent event = EVENT_NONE;
	if (FrameReady(nes))
	{
		event |= EVENT_NEW_FRAME;
	}
	if (AudioReady(nes))
	{
		event |= EVENT_AUDIO_BUFFER_FULL;
	}
	if (nes.system_clock == ppu_cycle)
	{
		event |= EVENT_UNTIL_TICKS;
	}
	return event;
}

EMSCRIPTEN_KEEPALIVE
uint32_t GetTotalPPUCycles()
{
	return nes.system_clock;
}

EMSCRIPTEN_KEEPALIVE
uint8_t* GetFrameBuffer()
{
	return get_framebuffer(&nes.ppu);
}

EMSCRIPTEN_KEEPALIVE
void SetKeys(uint8_t keys_in[8])
{
	Keys keys;
	keys.reg = 0x00;
	for (int i = 0; i < 8; i++)
	{
		if (keys_in[i])
		{
			keys.reg |= (1 << i);
		}
	}
	poll_keys(&nes.pad, keys);
}

float audio_buffer[4096];

EMSCRIPTEN_KEEPALIVE
float* GetAudioBuffer()
{
	return &audio_buffer[0];
}

EMSCRIPTEN_KEEPALIVE
uint32_t FlushAudioSamples()
{
	uint32_t ret = nes.apu.audio_pos;
	memcpy(audio_buffer, nes.apu.audio_buffer, ret * sizeof(float));
	nes.apu.audio_pos = 0;
	return ret;
}