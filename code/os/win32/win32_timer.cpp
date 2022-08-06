
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
    i64 frame_counts = end_frame.QuadPart - timer->begin_frame.QuadPart;
    os->time.wall_dt_for_frame = frame_counts  / (f32)timer->counts_per_second.QuadPart;
    os->time.wall_time += os->time.wall_dt_for_frame;
}
