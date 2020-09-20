#include "timer.h"


#ifdef PLATFORM_WINDOWS

static LARGE_INTEGER freq = { 0 };

void GetTime(timepoint* tp)
{
	QueryPerformanceCounter(&tp->tp);
}

float GetElapsedTimeMicro(timepoint* beg, timepoint* end)
{
	if (freq.QuadPart == 0)
	{
		QueryPerformanceFrequency(&freq);
	}

	return (float)(end->tp.QuadPart - beg->tp.QuadPart) / freq.QuadPart * 1000000;
}

float GetElapsedTimeMilli(timepoint* beg, timepoint* end)
{
	if (freq.QuadPart == 0)
	{
		QueryPerformanceFrequency(&freq);
	}

	return (float)(end->tp.QuadPart - beg->tp.QuadPart) / freq.QuadPart * 1000;
}

#endif

#ifdef PLATFORM_LINUX
// TODO
#endif