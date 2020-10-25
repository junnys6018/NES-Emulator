#include "Frontend/Renderer.h"
#include "Frontend/Gui.h"
#include "Frontend/Controller.h"

#include "Backend/nes.h"

#include "test_6502.h"
#include "test_2C02.h"
#include "Benchmarks.h"

#include "timer.h"
#include "FileDialog.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <SDL.h>

void my_audio_callback(void* userdata, Uint8* stream, int len)
{
	static float freq = 440; // in hz
	static int64_t time = 0; // in 1/spec.freq units of time

	float* my_stream = (float*)stream;
	for (int i = 0; i < len/sizeof(float); i++)
	{
		//my_stream[i] = 0.5 * ((float)rand() / RAND_MAX - 0.5f);
		my_stream[i] = 0.2f * sinf(2 * M_PI * freq * time / 44100);
		time++;
	}
}

void play_sound()
{
	SDL_AudioSpec spec, have;
	memset(&spec, 0, sizeof(spec));
	spec.freq = 44100;
	spec.format = AUDIO_F32;
	spec.channels = 1;
	spec.samples = 4096;
	spec.callback = my_audio_callback;
	spec.userdata = NULL;

	SDL_AudioDeviceID id = SDL_OpenAudioDevice(NULL, 0, &spec, &have, 0);

	SDL_PauseAudioDevice(id, 0);

	//SDL_CloseAudioDevice(id);
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

	//play_sound();

	//RunAll6502Tests();
	//RunAll2C02Tests();
	//RunAllBenchmarks();

	Nes nes;
	RendererBindNES(&nes);
	NESInit(&nes, NULL);

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
	timepoint beg, end;
	bool running = true;
	while (running)
	{
		GetTime(&beg);
		poll_keys(&nes.pad);

		if (controller.mode == MODE_PLAY)
		{
			clock_nes_frame(&nes);
		}

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
		GetTime(&end);

		total_time += GetElapsedTimeMilli(&beg, &end);
		if (++frame >= window)
		{
			controller.ms_per_frame = total_time / window;
			controller.fps = 1000.0f / controller.ms_per_frame;

			frame = 0;
			total_time = 0.0f;
		}

		float sleep_time = 16666 - GetElapsedTimeMicro(&beg, &end);
		if (sleep_time > 0)
		{
			SleepMicro((uint64_t)sleep_time);
		}
	}

	NESDestroy(&nes);
	RendererShutdown();

	SDL_Quit();

	exit(EXIT_SUCCESS);
}
