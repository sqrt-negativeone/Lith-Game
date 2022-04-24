/* date = April 22nd 2022 9:12 pm */

#ifndef GAME_EVENT_H
#define GAME_EVENT_H

// TODO(fakhri): this is more like a buffered commands than
// it is events, so probably I should rename it
enum Game_Event_Kind
{
    GameEventKind_DisplayMessage,
    GameEventKind_BurnCards,
    GameEventKind_Delay,
    GameEventKind_ChangeCurrentPlayer,
    GameEventKind_OpenDeclareMenu,
    GameEventKind_CloseDeclareMenu,
};

struct Game_Event
{
    Game_Event_Kind kind;
    char buffer[128];
    f32 duration;
    
    String8 string;
    v4 string_color;
    v3 string_position;
    Coordinate_Type coords_type;
    Card_Number declared_number;
    Game_Event *next;
};

struct Game_Event_Buffer
{
    Game_Event *first;
    Game_Event *last;
    M_Arena    *arena;
    Game_Event *free_list;
};

#endif //GAME_EVENT_H
