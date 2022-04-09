/* date = March 26th 2022 6:03 pm */

#ifndef OS_EVENT_H
#define OS_EVENT_H

typedef enum OS_EventKind
{
    OS_EventKind_Null,
    OS_EventKind_Press,
    OS_EventKind_Release,
    OS_EventKind_Text,
    OS_EventKind_MouseScroll,
    OS_EventKind_COUNT
}
OS_EventKind;

typedef U32 OS_Modifiers;
enum
{
    OS_Modifier_Ctrl  = (1<<0),
    OS_Modifier_Shift = (1<<1),
    OS_Modifier_Alt   = (1<<2),
};

typedef struct OS_Event OS_Event;
struct OS_Event
{
    OS_Event *next;
    OS_Event *prev;
    
    OS_EventKind kind;
    OS_Modifiers modifiers;
    OS_Key key;
    u32 character;
    v2 position;
    v2 scroll;
    String8 path;
};

typedef struct OS_EventList OS_EventList;
struct OS_EventList
{
    OS_Event *first;
    OS_Event *last;
    U64 count;
    
    OS_Event *free_events;
};

internal void OS_EventListPushBack(OS_EventList *list, OS_Event *event);
internal OS_Event *OS_MakeEvent(M_Arena *arena, OS_EventList *list);
internal void OS_EatEvent(OS_EventList *events, OS_Event *event);
internal void OS_EatAllEvents(OS_EventList *event_list);

#endif //OS_EVENT_H
