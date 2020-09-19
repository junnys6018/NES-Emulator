#include "timer.h"


#ifdef PLATFORM_WINDOWS

void GetTime(timepoint* tp)
{
	QueryPerformanceCounter(&tp->tp);
}

float GetElapsedTimeMicro(timepoint* beg, timepoint* end)
{
	static LARGE_INTEGER freq = { 0 };
	if (freq.QuadPart == 0)
	{
		QueryPerformanceFrequency(&freq);
	}

	return (float)(end->tp.QuadPart - beg->tp.QuadPart) / freq.QuadPart * 1000000;
}

#endif

#ifdef PLATFORM_LINUX
// TODO
#endif