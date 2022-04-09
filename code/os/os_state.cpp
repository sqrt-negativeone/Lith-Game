
internal void
OS_EndFrame()
{
    ++os->frame_index;
    os->time.game_time += os->time.game_dt_for_frame;
    OS_EatAllEvents(&os->events);
}

internal M_Arena *
OS_FrameArena()
{
    u64 arena_index = os->frame_index % ArrayCount(os->frame_arenas);
    M_Arena *result = os->frame_arenas[arena_index];
    return result;
}
