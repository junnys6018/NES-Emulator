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

void SleepMicro(uint64_t usec)
{
	HANDLE timer;
	LARGE_INTEGER ft;

	ft.QuadPart = -(10 * (LONGLONG)usec); // Convert to 100 nanosecond interval, negative value indicates relative time

	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
}

#endif

#ifdef PLATFORM_LINUX
// TODO
#endif