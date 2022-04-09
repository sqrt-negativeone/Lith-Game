
internal b32  
W32_TimerInit(W32_Timer *timer)
{
    b32 result = 0;
    
    if(QueryPerformanceFrequency(&timer->counts_per_second))
    {
        result = 1;
    }
    
    timer->is_sleep_granular = (timeBeginPeriod(1) == TIMERR_NOERROR);
    
    return result;
}

internal void 
W32_TimerBeginFrame(W32_Timer *timer)
{
    QueryPerformanceCounter(&timer->begin_frame);
}

internal void 
W32_TimerEndFrame(W32_Timer *timer)
{
    LARGE_INTEGER end_frame;
    
    QueryPerformanceCounter(&end_frame);
    
#if 1
    i64 elapsed_counts = end_frame.QuadPart - timer->begin_frame.QuadPart;
    i64 desired_counts = (i64)(os->time.game_dt_for_frame * timer->counts_per_second.QuadPart);
    i64 counts_to_wait = desired_counts - elapsed_counts;
    
    LARGE_INTEGER start_wait;
    LARGE_INTEGER end_wait;
    
    QueryPerformanceCounter(&start_wait);
    
    while(counts_to_wait > 0)
    {
        if(timer->is_sleep_granular)
        {
            f32 seconds_to_wait = (f32)(counts_to_wait) / timer->counts_per_second.QuadPart;
            DWORD milliseconds_to_sleep = (DWORD)(MilliSeconds(seconds_to_wait));
            if(milliseconds_to_sleep > 0)
            {
                Sleep(milliseconds_to_sleep);
            }
        }
        
        QueryPerformanceCounter(&end_wait);
        counts_to_wait -= end_wait.QuadPart - start_wait.QuadPart;
        start_wait = end_wait;
    }
#endif
    
    QueryPerformanceCounter(&end_frame);
    i64 frame_counts = end_frame.QuadPart - timer->begin_frame.QuadPart;
    os->time.wall_dt_for_frame = frame_counts  / (f32)timer->counts_per_second.QuadPart;
    os->time.wall_time += os->time.wall_dt_for_frame;
}
