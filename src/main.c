#include "Frontend/Renderer.h"
#include "Frontend/Gui.h"
#include "Frontend/Controller.h"

#include "Backend/nes.h"

#include "test_6502.h"
#include "test_2C02.h"
#include "Benchmarks.h"

#include "timer.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include <SDL.h>

float absf(float f)
{
	return f < 0.0f ? -f : f;
}

typedef struct
{
	Nes* nes;
	Controller* controller;

	// For DC cutoff filter
	float last_sample;
	float last_filter;

} Userdata;


void my_audio_callback(void* userdata, Uint8* stream, int len)
{
	float* my_stream = (float*)stream;
	Userdata* data = (Userdata*)userdata;
	Nes* nes = data->nes;
	Controller* controller = data->controller;
	if (controller->mode == MODE_PLAY)
	{
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
	else
	{
		memset(stream, 0, len);
	}
}

SDL_AudioDeviceID play_sound(Userdata* data)
{
	SDL_AudioSpec spec, have;
	memset(&spec, 0, sizeof(spec));
	spec.freq = SAMPLE_RATE;
	spec.format = AUDIO_F32;
	spec.channels = 1;
	spec.samples = 128;
	spec.callback = my_audio_callback;
	spec.userdata = data;

	SDL_AudioDeviceID id = SDL_OpenAudioDevice(NULL, 0, &spec, &have, 0);

	SDL_PauseAudioDevice(id, 0);

	return id;
}

int main(int argc, char** argv)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("Could not initialize SDL");
		exit(EXIT_FAILURE);
	}

	Controller controller = { .mode = MODE_NOT_RUNNING };
	RendererInit(&controller);
	AudioInit();
	Nes nes;
	NESInit(&nes, NULL);
	RendererBindNES(&nes);
	RendererDraw();

	if (argc == 2 && strcmp(argv[1], "--test") == 0)
	{
		RunAll6502Tests();
		RunAll2C02Tests();
		RunAllBenchmarks();

		RendererBindNES(&nes);
	}
	else if (argc == 2)
	{
		NESDestroy(&nes);
		if (NESInit(&nes, argv[1]) == 0)
		{
			controller.mode = MODE_PLAY;
		}
		else
		{
			NESInit(&nes, NULL);
		}
	}
	Userdata data = { .nes = &nes, .controller = &controller };
	controller.audio_id = play_sound(&data);

	//nes.pad.current_input.reg = 0x00;
	//for (int i = 0; i < 11199700; i++)
	//{
	//	clock_nes_cycle(&nes);
	//	clock_nes_cycle(&nes);
	//	clock_nes_cycle(&nes);
	//}

	// How many frames to average fps statistic over
	const int window = 15;
	float total_time = 0.0f;
	int frame = 0;

	SDL_Event event;
	bool running = true;
	while (running)
	{
		poll_keys(&nes.pad);

		while (SDL_PollEvent(&event) != 0)
		{
			GuiDispatchEvent(&event);
			if (controller.mode == MODE_STEP_THROUGH && event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_SPACE:
					clock_nes_instruction(&nes);
					break;
				case SDLK_f:
					clock_nes_frame(&nes);
					break;
				case SDLK_p:
					clock_nes_cycle(&nes);
					break;
				}
			}
			else if (event.type == SDL_QUIT)
			{
				running = false;
			}
		}

		RendererDraw();
	}

	SDL_CloseAudioDevice(controller.audio_id);
	NESDestroy(&nes);
	RendererShutdown();
	SDL_Quit();

	exit(EXIT_SUCCESS);
}
