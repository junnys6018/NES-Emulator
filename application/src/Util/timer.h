#ifndef TIMER_H
#define TIMER_H

#ifdef PLATFORM_WINDOWS
// This struct should be treated as opaque.
// I've exposed its definition so users can create timepoints on the stack
#include <Windows.h>
typedef struct
{
	LARGE_INTEGER tp;
} timepoint;
#endif

#ifdef PLATFORM_LINUX

#include <sys/time.h>
// This struct should be treated as opaque.
// I've exposed its definition so users can create timepoints on the stack
typedef struct timeval timepoint;

#endif

#include <stdint.h>

void get_time(timepoint* tp);
float get_elapsed_time_micro(timepoint* beg, timepoint* end);
float get_elapsed_time_milli(timepoint* beg, timepoint* end);

void sleep_micro(uint64_t usec);

#endif // ! TIMER_H
