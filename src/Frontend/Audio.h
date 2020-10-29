#ifndef AUDIO_H
#define AUDIO_H
#include <SDL.h>
#include "Backend/nes.h"
#include "Frontend/Controller.h"

typedef struct
{
	Nes* nes;

	// For DC cutoff filter
	float last_sample;
	float last_filter;

} Userdata;

SDL_AudioDeviceID init_sdl_audio(Userdata* data);

#endif // !AUDIO_H
