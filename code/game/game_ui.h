/* date = February 2nd 2022 1:28 am */

#ifndef GAME_UI_H
#define GAME_UI_H

enum GameMenuKind
{
    GameMenuKind_None,
    GameMenuKind_Main,
    GameMenuKind_JoinGame,
    GameMenuKind_JoinGameConfirmation,
    GameMenuKind_HostGame,
    GameMenuKind_EnterUsername,
    GameMenuKind_UsernameConfirmation,
    GameMenuKind_JoinedPlayers,
    GameMenuKind_NetworkError,
    GameMenuKind_InvalidUsername,
    
    GameMenuKind_HostInGameError,
    GameMenuKind_Count,
};

struct Game_Menu
{
    b32 is_active;
    f32 presence;
    f32 presence_change_speed;
};

enum Game_UI_InputFieldKind
{
    InputFieldKind_HostSessionName,
    InputFieldKind_GameID,
    InputFieldKind_PlayerUsername,
    InputFieldKind_Count,
};


struct Game_UI_InputField
{
    u8 buffer[20];
    u32 size;
    
    f32 cursor_x_offset;
    f32 cursor_target_x_offset;
    f32 cursor_x_offset_speed;
    
    u32 cursor_index;
};

struct Game_UI
{
    Render_Context *render_context;
    struct Controller *controller;
    Font_Kind active_font;
    Coordinate_Type active_coordinates;
    
    u32 active_widget;
    u32 hot_widget;
    
    f32 active_transition;
    f32 active_transition_speed;
    
    f32 hot_transition;
    f32 hot_transition_speed;
    
    u32 current_widget;
    
    Game_Menu menus[GameMenuKind_Count];
    GameMenuKind active_menu;
    GameMenuKind current_menu;
    
    Game_UI_InputField input_fields[InputFieldKind_Count];
};

#endif //GAME_UI_H
