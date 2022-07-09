/* date = February 2nd 2022 1:28 am */

#ifndef GAME_UI_H
#define GAME_UI_H

enum GameMenuKind
{
    GameMenuKind_None,
    GameMenuKind_Main,
    GameMenuKind_JoinGame,
    GameMenuKind_Count,
};

struct Game_Menu
{
    b32 accept_input;
    f32 presence;
    f32 presence_change_speed;
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
    u32 widgets_count;
    
    Game_Menu menus[GameMenuKind_Count];
    GameMenuKind active_menu;
    GameMenuKind current_menu;
    
};

#endif //GAME_UI_H
