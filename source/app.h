/* date = January 15th 2022 1:05 am */

#ifndef APP_H
#define APP_H

#include "buffer.h"
#include "ui.h"
#include "renderer.h"
#include "entity.h"
#include "network_shared/host_info.h"

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
    SESSION_FLAG_HOSTING_GAME                 = (1 << 0),
    SESSION_FLAG_SERVER_DOWN                  = (1 << 1),
    SESSION_FLAG_TRYING_CONNECT_GAME          = (1 << 2),
    SESSION_FLAG_CONNECTED_TO_GAME            = (1 << 3),
    SESSION_FLAG_FAILED_CONNECT_GAME          = (1 << 4),
    SESSION_FLAG_TRYING_JOIN_GAME             = (1 << 5),
    SESSION_FLAG_FAILED_JOIN_GAME             = (1 << 6),
    SESSION_FLAG_JOINED_GAME                  = (1 << 7),
    SESSION_FLAG_HOST_SPLITTING_DECK          = (1 << 8),
    SESSION_FLAG_HOST_FINISHED_SPLITTING_DECK = (1 << 8),
};


struct Residency
{
    u32 entity_indices[DECK_CARDS_COUNT];
    u32 entity_count;
    u32 controlling_player_id;
    
    b32 is_horizonal;
    b32 is_stacked;
};

struct Hosts_Storage
{
    // TODO(fakhri): how many do we need to display?
    b32 is_fetching;
    Host_Info hosts[10];
    u32 hosts_count;
};


struct Player
{
    b32 joined;
    s8 username;
    Card_Residency assigned_residency;
};


struct Game_Session
{
    Player players[MAX_PLAYER_COUNT];
    u32 players_joined_so_far;
    // NOTE(fakhri): indicate if we are the ones hosting the game
    u32 flags;
    Hosts_Storage hosts_storage;
    u32 current_player_id;
    
    u32 my_player_id;
};

struct Game_State
{
    Game_Mode game_mode;
    UI_Context ui_context;
    Rendering_Context rendering_context;
    Game_Session game_session;
    Controller controller;
    
    Entity entities[1024];
    u32 entity_count;
    
    u32 card_pressed_index;
    Residency residencies[Card_Residency_Count];
    
    // NOTE(fakhri): menu stuff
    Buffer host_address_buffer;
    Buffer username_buffer;
    
    b32 should_burn_cards;
    f32 time_to_burn_cards;
    
    Buffer message_to_display;
    f32 message_duration;
    
    v2 world_dim;
};

struct Compile_Shader_Result
{
    b32 is_valid;
    GLuint program;
};

#endif //APP_H
