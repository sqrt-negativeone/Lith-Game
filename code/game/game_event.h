/* date = April 22nd 2022 9:12 pm */

#ifndef GAME_EVENT_H
#define GAME_EVENT_H


enum Game_Event_Kind
{
    GameEventKind_DisplayMessage,
    GameEventKind_BurnCards,
    GameEventKind_Delay,
};

struct Game_Event
{
    Game_Event_Kind kind;
    char buffer[128];
    String8 string;
    f32 duration;
    Game_Event *next;
};

struct Game_Event_Buffer
{
    Game_Event *first;
    Game_Event *last;
    
    Game_Event *free_list;
};

#endif //GAME_EVENT_H
