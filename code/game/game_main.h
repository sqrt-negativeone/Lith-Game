/* date = January 15th 2022 1:05 am */

#ifndef GAME_MAIN_H
#define GAME_MAIN_H

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
    GameMode_GAME,
    GameMode_TestingNetworking,
    
    GameMode_MENU_BEGIN,
    // NOTE(fakhri): begin menu modes
    GameMode_MENU_Username,
    GameMode_MENU_MAIN,
    GameMode_MENU_JOIN_GAME,
    GameMode_MENU_HOST_GAME,
    GameMode_MENU_WAITING_PLAYERS,
    GameMode_MENU_END,
    
};

typedef u32 GameStateFlags;
enum
{
    StateFlag_HostingGame             = (1 << 0),
    StateFlag_ServerDown              = (1 << 1),
    StateFlag_TryingConnectGame       = (1 << 2),
    StateFlag_ConnectedToGame         = (1 << 3),
    StateFlag_FailedConnectToGame     = (1 << 4),
    StateFlag_TryingJoinGame          = (1 << 5),
    StateFlag_FailedJoinGame          = (1 << 6),
    StateFlag_JoinedGame              = (1 << 7),
    StateFlag_WaitingForCards         = (1 << 8),
    StateFlag_ReceivedCards           = (1 << 9),
    StateFlag_ShouldOpenDeclaringMenu = (1 << 10),
    StateFlag_ShouldBurnCards         = (1 << 11),
    StateFlag_ShouldDeclareCard       = (1 << 12),
    StateFlag_PlaySelectedCards       = (1 << 13),
    StateFlag_QuestionCredibility     = (1 << 14),
    StateFlag_PlayedCardThisFrame     = (1 << 15),
};

enum GameStateFlagsGroup
{
    GameStateFlagsGroup_NoSelect = StateFlag_ShouldDeclareCard | StateFlag_ShouldBurnCards | StateFlag_ShouldOpenDeclaringMenu,
    GameStateFlagsGroup_ValidOnlyForOneFrame = StateFlag_PlaySelectedCards | StateFlag_QuestionCredibility,
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
    String8 username;
    ResidencyKind assigned_residency_kind;
};

struct Game_State
{
    Game_Mode game_mode;
    GameStateFlags flags;
    Render_Context render_context;
    Frensh_Suited_Cards_Texture frensh_deck;
    UI_Context ui_context;
    Player players[MAX_PLAYER_COUNT];
    u32 players_joined_so_far;
    PlayerID prev_player_id;
    PlayerID curr_player_id;
    PlayerID my_player_id;
    Controller controller;
    Game_Command_Buffer command_buffer;
    Entity entities[1024];
    u32 entity_count;
    Residency residencies[ResidencyKind_Count];
    // NOTE(fakhri): menu stuff
    Buffer host_address_buffer;
    Buffer username_buffer;
    f32 time_scale_factor;
    EntityID highest_entity_under_cursor;
    u32 selection_limit;
    u32 prev_played_cards_count;
    u32 selection_count;
    Card_Number declared_number;
    EntityID card_type_to_entity_id_map[Category_Count][Card_Number_Count];
};

#endif //GAME_MAIN_H
