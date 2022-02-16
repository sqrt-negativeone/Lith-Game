/* date = January 15th 2022 1:05 am */

#ifndef APP_H
#define APP_H

#include "buffer.h"
#include "ui.h"
#include "renderer.h"

#define PLAYERS_COUNT 4
#define CARDS_PER_PLAYER 10
#define DECK_CARDS_COUNT (PLAYERS_COUNT * CARDS_PER_PLAYER)

struct Input
{
    b32 pressed;
    b32 released;
};

struct Controller
{
    Input move_left;
    Input move_right;
    Input move_up;
    Input move_down;
    
    Input right_mouse;
    Input left_mouse;
    
    Input confirm;
    Input escape_key;
    Input toggle_fullscreen;
};

enum Game_Mode
{
    Game_Mode_GAME,
    
    Game_Mode_MENU_BEGIN,
    // NOTE(fakhri): begin menu modes
    Game_Mode_MENU_USERNAME,
    Game_Mode_MENU_MAIN,
    Game_Mode_MENU_JOIN_GAME,
    Game_Mode_MENU_HOST_GAME,
    Game_Mode_MENU_WAITING_PLAYERS,
    Game_Mode_MENU_END,
    
};


enum Game_Session_Flags
{
    SESSION_FLAG_HOSTING_GAME        = (1 << 0),
    SESSION_FLAG_TRYING_CONNECT_GAME = (1 << 2),
    SESSION_FLAG_CONNECTED_TO_GAME   = (1 << 3),
    SESSION_FLAG_FAILED_CONNECT_GAME = (1 << 4),
    SESSION_FLAG_TRYING_JOIN_GAME    = (1 << 5),
    SESSION_FLAG_FAILED_JOIN_GAME    = (1 << 6),
    SESSION_FLAG_JOINED_GAME         = (1 << 7),
};


struct Game_Session
{
    s8 players[PLAYERS_COUNT];
    u32 players_joined_sofar;
    // NOTE(fakhri): indicate if we are the ones hosting the game
    u32 session_state_flags;
};

#if 1
enum Entity_Type
{
    Entity_Type_Null_Entity,
    Entity_Type_Cursor_Entity, // NOTE(fakhri): always following mouse
    Entity_Type_Entity_Card,
    Entity_Type_Entity_Companion,
};

enum Card_Residency
{
    Card_Residency_None,
    Card_Residency_Up,
    Card_Residency_Down,
    Card_Residency_Left,
    Card_Residency_Right,
    Card_Residency_Table,
    Card_Residency_Burnt,
    
    Card_Residency_Count,
};

#define CARD_WIDTH  (2 * 3.5f)
#define CARD_HEIGHT (2 * 5.45f)
#define CARD_X_GAP (0.420f)
#define CARD_Y_GAP (-5.0f)


struct Entity
{
    Entity_Type type;
    v2 residency_pos;
    v2 center_pos;
    v2 target_pos;
    
    f32 y_angle;
    f32 target_y_angle;
    f32 dy_angle;
    
    v2 current_dimension;
    v2 target_dimension;
    f32 dDimension;
    
    v2 velocity;
    
    // TODO(fakhri): we can have flags here instead, then each type of entity can have its own flags
    b32 is_under_cursor;
    b32 is_pressed;
    
    u32 followed_entity_index;
    v2  offset_in_follwed_entity;
    
    Card_Type card_type;
    
    Card_Residency residency;
    GLuint texture;
};

struct Residency
{
    u32 entity_indices[40];
    u32 entity_count;
};

#else
struct Card
{
    v2 size;
    v2 postion;
};

struct Number
{
    u32 value;
    u32 followed_card;
};
#endif


struct Game_State
{
    Game_Mode game_mode;
    UI_Context ui_context;
    Rendering_Context rendering_context;
    Game_Session game_session;
    
    Entity entities[256];
    u32 entity_count;
    
    Residency entity_residencies[Card_Residency_Count];
};

struct Compile_Shader_Result
{
    b32 is_valid;
    GLuint program;
};

#endif //APP_H
