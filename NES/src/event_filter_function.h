#ifndef EVENT_FILTER_FUNCTION_H
#define EVENT_FILTER_FUNCTION_H

#include <SDL.h>

typedef struct 
{
	int size;
	Uint32 event_types[32];
}EventTypeList;


static int event_whitelist(void* userdata, SDL_Event* event)
{
	if (userdata == NULL)
	{
		return 0;
	}

	EventTypeList list = *(EventTypeList*)userdata;
	Uint32 type = event->type;
	for (int i = 0; i < list.size; i++)
	{
		if (type == list.event_types[i])
			return 1;
	}

	return 0;
}

static int reset_filter_event(void* userdata, SDL_Event* event)
{
	return 1;
}
#endif
