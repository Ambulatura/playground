internal b32 Win32TimerInitialize(Win32Timer* timer)
{
	b32 result = false;
	
	if (QueryPerformanceFrequency(&timer->frequency)) {
		result = true;
	}
	
	timer->is_sleep_granular = (timeBeginPeriod(1) == TIMERR_NOERROR);

	return result;
}

internal LARGE_INTEGER Win32GetCounts()
{
	LARGE_INTEGER result;
	QueryPerformanceCounter(&result);
	return result;
}

internal i32 Win32GetCountsElapsed(LARGE_INTEGER begin, LARGE_INTEGER end)
{
	i32 result = (i32)end.QuadPart - (i32)begin.QuadPart;
	return result;
}

internal f32 Win32GetSecondsElapsed(LARGE_INTEGER begin, LARGE_INTEGER end, LARGE_INTEGER frequency)
{
	f32 result = (f32)Win32GetCountsElapsed(begin, end) / (f32)frequency.QuadPart;
	return result;
}

internal void Win32TimerBeginFrame(Win32Timer* timer)
{
	timer->begin_counts = Win32GetCounts();
}

internal void Win32TimerEndFrame(Win32Timer* timer, f32 target_seconds_per_frame)
{
	timer->end_counts = Win32GetCounts();

	i32 elapsed_counts = Win32GetCountsElapsed(timer->begin_counts, timer->end_counts);
	i32 target_counts = (i32)(target_seconds_per_frame * timer->frequency.QuadPart);
	i32 counts_to_wait = target_counts - elapsed_counts;

	LARGE_INTEGER end_wait_counts;
	LARGE_INTEGER start_wait_counts = Win32GetCounts();

	while (counts_to_wait > 0) {
		if (timer->is_sleep_granular) {
			DWORD sleep_in_milliseconds = (DWORD)(1000.0f * ((f32)counts_to_wait / timer->frequency.QuadPart));
			if (sleep_in_milliseconds > 0) {
				Sleep(sleep_in_milliseconds);
			}
		}

		end_wait_counts = Win32GetCounts();
		counts_to_wait -= ((i32)end_wait_counts.QuadPart - (i32)start_wait_counts.QuadPart);
		start_wait_counts = end_wait_counts;
	}
	
}
