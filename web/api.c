#include "emscripten.h"
#include "nes.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

EMSCRIPTEN_KEEPALIVE
void EmulateFrame()
{
	clock_nes_frame(&nes);
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