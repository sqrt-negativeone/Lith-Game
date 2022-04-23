
internal Game_Event *
GameEvent_PushEvent(Game_Event_Buffer *event_buffer)
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
        game_event = PushStructZero(event_buffer->arena, Game_Event);
    }
    
    if (!event_buffer->first)
    {
        event_buffer->first = event_buffer->last = game_event;
    }
    else
    {
        event_buffer->last->next = game_event;
        event_buffer->last       = game_event;
    }
    return game_event;
}

internal Game_Event *
GameEvent_PushEvent_DisplayMessag(Game_Event_Buffer *event_buffer, String8 string, v4 string_color, v3 string_position, Coordinate_Type coords_type, f32 duration)
{
    Game_Event *game_event = GameEvent_PushEvent(event_buffer);
    Assert(game_event);
    Assert(string.len <= ArrayCount(game_event->buffer));
    
    game_event->string.cstr = game_event->buffer;
    MemoryCopy(game_event->string.str, string.str, string.len);
    game_event->string.len = string.len;
    
    game_event->string_position = string_position;
    game_event->kind            = GameEventKind_DisplayMessage;
    game_event->duration        = duration;
    game_event->string_color    = string_color;
    game_event->coords_type     = coords_type;
    
    return game_event;
}

internal Game_Event *
GameEvent_PushEvent_BurnCards(Game_Event_Buffer *event_buffer)
{
    Game_Event *game_event = GameEvent_PushEvent(event_buffer);
    Assert(game_event);
    
    game_event->kind     = GameEventKind_BurnCards;
    game_event->duration = 0;
    
    return game_event;
}

internal Game_Event *
GameEvent_PushEvent_Delay(Game_Event_Buffer *event_buffer, f32 duration)
{
    Game_Event *game_event = GameEvent_PushEvent(event_buffer);
    Assert(game_event);
    
    game_event->kind     = GameEventKind_Delay;
    game_event->duration = duration;
    
    return game_event;
}

internal Game_Event *
GameEvent_PushEvent_ChangeCurrentPlayer(Game_Event_Buffer *event_buffer)
{
    Game_Event *game_event = GameEvent_PushEvent(event_buffer);
    Assert(game_event);
    
    game_event->kind = GameEventKind_ChangeCurrentPlayer;
    
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
