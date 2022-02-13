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
    Entity_Type_Entity_Card_Number,
};

struct Entity
{
    Entity_Type type;
    v2 original_pos;
    v2 center_pos;
    v2 target_pos;
    
    v2 original_dimension;
    v2 current_dimension;
    v2 target_dimension;
    f32 dDimension;
    
    v2 velocity;
    
    b32 is_under_cursor;
    b32 is_pressed;
    
    f32 following_trigger_distance;
    u32 followed_entity_index;
    
    u32 card_number;
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
};

struct Compile_Shader_Result
{
    b32 is_valid;
    GLuint program;
};

#endif //APP_H
