#ifndef WIN32_TIMER_H

struct Win32Timer
{
	LARGE_INTEGER frequency;
	LARGE_INTEGER begin_counts;
	LARGE_INTEGER end_counts;
	b32 is_sleep_granular;
};

internal LARGE_INTEGER Win32GetCounts();
internal i32 Win32GetCountsElapsed(LARGE_INTEGER begin, LARGE_INTEGER end);
internal f32 Win32GetSecondsElapsed(LARGE_INTEGER begin, LARGE_INTEGER end, LARGE_INTEGER frequency);

internal b32 Win32TimerInitialize(Win32Timer* timer);
internal void Win32TimerBeginFrame(Win32Timer* timer);
internal void Win32TimerEndFrame(Win32Timer* timer, f32 target_seconds_per_frame);

#define WIN32_TIMER_H
#endif
