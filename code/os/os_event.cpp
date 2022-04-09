internal void 
OS_EventListPushBack(OS_EventList *list, OS_Event *event)
{
    if (list->first == 0)
    {
        // NOTE(fakhri): empty list
        list->first = list->last = event;
        event->next = 0;
        event->prev = 0;
    }
    else
    {
        OS_Event *last = list->last;
        last->next = event;
        event->prev = last;
        event->next = 0;
        list->last = event;
    }
}

internal OS_Event *
OS_MakeEvent(M_Arena *arena, OS_EventList *event_list)
{
    OS_Event *result = 0;
    if (event_list->free_events)
    {
        result = event_list->free_events;
        event_list->free_events = result->next;
    }
    
    if (result == 0)
    {
        result = PushStruct(arena, OS_Event);
    }
    
    if (result)
    {
        MemoryZeroStruct(result);
    }
    
    return result;
}

internal void
OS_EatEvent(OS_EventList *event_list, OS_Event *event)
{
    OS_Event *next = event->next;
    OS_Event *prev = event->prev;
    
    if (event_list->first == event)
    {
        event_list->first = event->next;
        event_list->first->prev = 0;
    }
    else if (event_list->last == event)
    {
        event_list->last = event->prev;
        event_list->last->next = 0;
    }
    else
    {
        prev->next = next;
        next->prev = prev;
    }
    
    event_list->count -= 1;
    
    // NOTE(fakhri): add the event to the free list
    event->prev = 0;
    event->next = event_list->free_events;
    event_list->free_events = event;
}

internal void
OS_EatAllEvents(OS_EventList *event_list)
{
    OS_Event *first = event_list->first;
    OS_Event *last  = event_list->last;
    // NOTE(fakhri): add all the events to the free events list
    if (first)
    {
        last->next = event_list->free_events;
        event_list->free_events = first;
        
        event_list->count = 0;
        event_list->first = 0;
        event_list->last  = 0;
    }
}
