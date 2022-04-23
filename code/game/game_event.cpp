
internal Game_Event *
GameEvent_PushEvent(M_Arena *arena, Game_Event_Buffer *event_buffer)
{
    Game_Event *game_event = 0;
    if (event_buffer->free_list)
    {
        game_event = event_buffer->free_list;
        event_buffer->free_list = game_event->next;
        MemoryZero(game_event, sizeof(Game_Event));
    }
    
    if (!game_event)
    {
        game_event = PushStructZero(arena, Game_Event);
    }
    
    if (!event_buffer->first)
    {
        event_buffer->first = event_buffer->last = game_event;
    }
    else
    {
        event_buffer->last->next = game_event;
        event_buffer->last = game_event;
    }
    return game_event;
}

internal Game_Event *
GameEvent_PushDisplayMessageEvent(M_Arena *arena, Game_Event_Buffer *event_buffer, String8 string, f32 duration)
{
    Game_Event *game_event = GameEvent_PushEvent(arena, event_buffer);
    Assert(game_event);
    Assert(string.len <= ArrayCount(game_event->buffer));
    
    game_event->string.cstr = game_event->buffer;
    MemoryCopy(game_event->string.str, string.str, string.len);
    game_event->string.len = string.len;
    
    game_event->kind = GameEventKind_DisplayMessage;
#if 1
    game_event->duration = duration;
#endif
    
    return game_event;
}

internal Game_Event *
GameEvent_PushBurnCardsEvent(M_Arena *arena, Game_Event_Buffer *event_buffer)
{
    Game_Event *game_event = GameEvent_PushEvent(arena, event_buffer);
    Assert(game_event);
    
    game_event->kind = GameEventKind_BurnCards;
    game_event->duration = 0;
    
    return game_event;
}

internal Game_Event *
GameEvent_PushDelayEvent(M_Arena *arena, Game_Event_Buffer *event_buffer, f32 duration)
{
    Game_Event *game_event = GameEvent_PushEvent(arena, event_buffer);
    Assert(game_event);
    
    game_event->kind = GameEventKind_Delay;
    game_event->duration = duration;
    
    return game_event;
}

internal void
GameEvent_EatFirtEvent(Game_Event_Buffer *event_buffer)
{
    Assert(event_buffer->first);
    Game_Event *first = event_buffer->first;
    if (first)
    {
        event_buffer->first = first->next;
        first->next = event_buffer->free_list;
        event_buffer->free_list = first;
    }
}
