#ifndef AUDIO_H
#define AUDIO_H
#include <SDL.h>
// Synchronous sound queue, code adapted from http://www.slack.net/~ant/

void WriteSamples(const float* in, int count);
void InitSDLAudio();
void ShutdownSDLAudio();

#endif // !AUDIO_H
