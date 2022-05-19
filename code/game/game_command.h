/* date = April 22nd 2022 9:12 pm */

#ifndef GAME_COMMAND_H
#define GAME_COMMAND_H

// TODO(fakhri): this is more like a buffered commands than
// it is events, so probably I should rename it
enum Game_Command_Kind
{
    GameCommandKind_DisplayMessage,
    GameCommandKind_BurnCards,
    GameCommandKind_Delay,
    GameCommandKind_OpenDeclareMenu,
};

struct Game_Command
{
    Game_Command_Kind kind;
    char buffer[128];
    f32 duration;
    
    String8 string;
    v4 string_color;
    v3 string_position;
    Coordinate_Type coords_type;
    Card_Number declared_number;
    Game_Command *next;
};

struct Game_Command_Buffer
{
    Game_Command *first;
    Game_Command *last;
    M_Arena      *arena;
    Game_Command *free_list;
};

#endif //GAME_COMMAND_H
