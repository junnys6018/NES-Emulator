#ifndef TIMER_H
#define TIMER_H

#ifdef PLATFORM_WINDOWS
// This struct should be treated as opaque.
// I've exposed its definition so users can create timepoints themselves
#include <Windows.h>
typedef struct
{
	LARGE_INTEGER tp;
} timepoint;
#endif

#ifdef PLATFORM_LINUX
// TODO
#endif


void GetTime(timepoint* tp);
float GetElapsedTimeMicro(timepoint* beg, timepoint* end);
float GetElapsedTimeMilli(timepoint* beg, timepoint* end);

#endif // ! TIMER_H
