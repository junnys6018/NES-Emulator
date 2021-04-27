#ifndef CHANNEL_ENABLE_MODEL_H
#define CHANNEL_ENABLE_MODEL_H
#include <stdbool.h>

// Flags used to disable each of the channels, for debugging
typedef struct
{
	bool SQ1;
	bool SQ2;
	bool TRI;
	bool NOISE;
	bool DMC;
} ChannelEnableModel;

#endif
