#include "Audio.h"
#include "Frontend/Controller.h"

#include <string.h>
#include <stdio.h>

float absf(float f)
{
	return f < 0.0f ? -f : f;
}

void audio_callback(void* userdata, Uint8* stream, int len)
{
	float* my_stream = (float*)stream;
	Userdata* data = (Userdata*)userdata;
	Nes* nes = data->nes;

	for (int i = 0; i < len / sizeof(float); i++)
	{
		// Clock until an audio sample is ready
		while (!clock_nes_cycle(nes));

		float sample = apu_filtered_sample(&nes->apu);

		float filtered = sample - data->last_sample + 0.995f * data->last_filter;
		data->last_sample = sample;
		data->last_filter = filtered;
		my_stream[i] = filtered;

		if (!(absf(my_stream[i]) <= 1.0f))
		{
			my_stream[i] = 0.0f;
			printf("[ERROR] sound\n");
		}
	}
}

SDL_AudioDeviceID init_sdl_audio(Userdata* data)
{
	SDL_AudioSpec spec, have;
	memset(&spec, 0, sizeof(spec));
	spec.freq = SAMPLE_RATE;
	spec.format = AUDIO_F32;
	spec.channels = 1;
	spec.samples = 256;
	spec.callback = audio_callback;
	spec.userdata = data;

	SDL_AudioDeviceID id = SDL_OpenAudioDevice(NULL, 0, &spec, &have, 0);

	return id;
}