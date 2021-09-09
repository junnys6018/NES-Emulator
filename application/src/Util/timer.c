#include "timer.h"

#ifdef PLATFORM_WINDOWS

static LARGE_INTEGER freq = { 0 };

void get_time(timepoint* tp)
{
	QueryPerformanceCounter(&tp->tp);
}

float get_elapsed_time_micro(timepoint* beg, timepoint* end)
{
	if (freq.QuadPart == 0)
	{
		QueryPerformanceFrequency(&freq);
	}

	return (float)(end->tp.QuadPart - beg->tp.QuadPart) / freq.QuadPart * 1000000;
}

float get_elapsed_time_milli(timepoint* beg, timepoint* end)
{
	if (freq.QuadPart == 0)
	{
		QueryPerformanceFrequency(&freq);
	}

	return (float)(end->tp.QuadPart - beg->tp.QuadPart) / freq.QuadPart * 1000;
}

void sleep_micro(uint64_t usec)
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

#include <stddef.h>

void get_time(timepoint* tp)
{
	gettimeofday(tp, NULL);
}

float get_elapsed_time_micro(timepoint* beg, timepoint* end)
{
	long beg_time = 1000000 * beg->tv_sec + beg->tv_usec;
	long end_time = 1000000 * end->tv_sec + end->tv_usec;

	return end_time - beg_time;
}
float get_elapsed_time_milli(timepoint* beg, timepoint* end)
{
	return get_elapsed_time_micro(beg,end) / 1000.0f;
}

#include <unistd.h>
void sleep_micro(uint64_t usec)
{
	usleep(usec);
}
#endif