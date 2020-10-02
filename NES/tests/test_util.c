#include "test_util.h"
#include <stdlib.h>
#include "event_filter_function.h"

Uint32 on_render_callback(Uint32 interval, void* param)
{
	SDL_Event event;
	SDL_UserEvent userevent;

	userevent.type = SDL_USEREVENT;
	userevent.code = 0;
	userevent.data1 = NULL;
	userevent.data2 = NULL;

	event.type = SDL_USEREVENT;
	event.user = userevent;

	SDL_PushEvent(&event);

	return interval;
}

void EmulateUntilHalt(Nes* nes, int instructions_per_frame)
{
	// Update screen every 16ms
	SDL_TimerID tid = SDL_AddTimer(16, on_render_callback, NULL);

	// Filter away events that are not used
	EventTypeList list = { .size = 2,.event_types = {SDL_USEREVENT, SDL_QUIT} };
	SDL_SetEventFilter(event_whitelist, &list);

	// Used to detect if cpu has halted
	uint16_t old_PC[32];

	int instructions_done = 0;

	SDL_Event event;
	while (true)
	{
		if (instructions_done < instructions_per_frame)
		{
			// Perform 5000 instructions at once
			for (int i = 0; i < 5000; i++)
			{
				clock_nes_instruction(nes);
				old_PC[i % 32] = nes->cpu.PC;
			}
			instructions_done += 5000;

			bool halted = true;
			for (int i = 0; i < 32; i++)
			{
				halted = halted && (old_PC[i] == nes->cpu.PC);
			}
			if (halted)
			{
				break;
			}
		}

		// Poll Events
		while (SDL_PollEvent(&event) != 0)
		{
			if (event.type == SDL_QUIT)
			{
				exit(EXIT_SUCCESS);
			}
			else if (event.type == SDL_USEREVENT && event.user.code == 0)
			{
				RendererDraw();
				instructions_done = 0;
			}
		}

	}
	SDL_RemoveTimer(tid);
	SDL_SetEventFilter(reset_filter_event, NULL);
}
