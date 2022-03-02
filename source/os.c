
internal b32
OS_EventIsMouse(OS_Event *event)
{
    return event->type > OS_EventType_MouseStart && event->type < OS_EventType_MouseEnd;
}

internal b32
OS_CompareEvents(OS_Event a, OS_Event b)
{
    b32 result = 0;
    if(a.type == b.type &&
       a.key == b.key &&
       a.mouse_button == b.mouse_button &&
       a.gamepad_button == b.gamepad_button &&
       a.character == b.character &&
       a.modifiers == b.modifiers)
    {
        result = 1;
    }
    return result;
}

internal OS_Event
OS_KeyPressEvent(Key key, KeyModifiers modifiers)
{
    OS_Event event = {0};
    event.type = OS_EventType_KeyPress,
    event.key = key;
    event.modifiers = modifiers;
    return event;
}

internal OS_Event
OS_KeyReleaseEvent(Key key, KeyModifiers modifiers)
{
    OS_Event event = {0};
    event.type = OS_EventType_KeyRelease,
    event.key = key;
    event.modifiers = modifiers;
    return event;
}

internal OS_Event
OS_CharacterInputEvent(u64 character)
{
    OS_Event event = {0};
    event.type = OS_EventType_CharacterInput,
    event.character = character;
    return event;
}

internal OS_Event
OS_MouseMoveEvent(v2 position, v2 delta)
{
    OS_Event event = {0};
    event.type = OS_EventType_MouseMove,
    event.position = position;
    event.delta = delta;
    return event;
}

internal OS_Event
OS_MousePressEvent(MouseButton button, v2 position)
{
    OS_Event event = {0};
    event.type = OS_EventType_MousePress,
    event.mouse_button = button;
    event.position = position;
    return event;
}

internal OS_Event
OS_MouseReleaseEvent(MouseButton mouse_button, v2 position)
{
    OS_Event event = {0};
    event.type = OS_EventType_MouseRelease,
    event.mouse_button = mouse_button;
    event.position = position;
    return event;
}

internal OS_Event
OS_MouseScrollEvent(v2 delta, KeyModifiers modifiers)
{
    OS_Event event = {0};
    event.type = OS_EventType_MouseScroll,
    event.scroll = delta;
    event.modifiers = modifiers;
    return event;
}

internal b32
OS_GetNextEvent(OS_Event **event)
{
    b32 result = 0;
    HardAssert(os != 0);
    u64 start_index = 0;
    OS_Event *new_event = 0;
    if(*event)
    {
        start_index = (*event - os->events) + 1;
    }
    for(u64 i = start_index; i < os->event_count; ++i)
    {
        if(os->events[i].type != OS_EventType_Null)
        {
            new_event = os->events+i;
            break;
        }
    }
    *event = new_event;
    result = new_event != 0;
    return result;
}

internal void
OS_EatEvent(OS_Event *event)
{
    event->type = OS_EventType_Null;
}

// NOTE(rjf): Only called by platform layers. Do not call in app.
internal void
OS_BeginFrame(void)
{
    os->pump_events = 0;
}

// NOTE(rjf): Only called by platform layers. Do not call in app.
internal void
OS_EndFrame(void)
{
    os->game_time += os->game_dt;
}

internal void
OS_PushEvent(OS_Event event)
{
    HardAssert(os != 0);
    
    if(os->event_count < ArrayCount(os->events))
    {
        os->events[os->event_count++] = event;
    }
}

internal void
OS_EmptyEvents()
{
    HardAssert(os != 0);
    os->event_count = 0;
    os->controller = {};
}
