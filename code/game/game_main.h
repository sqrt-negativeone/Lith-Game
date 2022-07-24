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


typedef u32 GameStateFlags;
enum
{
    StateFlag_ShouldOpenDeclaringMenu = (1 << 0),
    StateFlag_ShouldBurnCards         = (1 << 1),
    StateFlag_ShouldDeclareCard       = (1 << 2),
    StateFlag_PlaySelectedCards       = (1 << 3),
    StateFlag_QuestionCredibility     = (1 << 4),
    StateFlag_TryingJoinGame          = (1 << 5),
    StateFlag_FailedJoinGame          = (1 << 6),
    StateFlag_JoinedGame              = (1 << 7),
    StateFlag_WaitingForCards         = (1 << 8),
    StateFlag_ReceivedCards           = (1 << 9),
    StateFlag_PlayedCardThisFrame     = (1 << 10),
    StateFlag_HostingGame             = (1 << 11),
    StateFlag_UsernameValid           = (1 << 12),
    StateFlag_UsernameInvalid         = (1 << 13),
    StateFlag_GameStarted             = (1 << 14),
    StateFlag_CantHost                = (1 << 15),
    StateFlag_HostDown                = (1 << 16),
    StateFlag_PlayerDisconnected      = (1 << 17),
    StateFlag_NoInput                 = (1 << 18),
    StateFlag_EndGame                 = (1 << 19),
};

enum GameStateFlagsGroup
{
    GameStateFlagsGroup_NoInput = (StateFlag_NoInput           | 
                                   StateFlag_ShouldDeclareCard | 
                                   StateFlag_ShouldBurnCards   | 
                                   StateFlag_ShouldOpenDeclaringMenu),
    GameStateFlagsGroup_ValidOnlyForOneFrame = StateFlag_PlaySelectedCards | StateFlag_QuestionCredibility,
};

struct Player
{
    b32 joined;
    String8 username;
    ResidencyKind assigned_residency_kind;
};

struct Game_State
{
    String game_id;
    Game_UI ui;
    GameStateFlags flags;
    Render_Context render_context;
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
    f32 time_scale_factor;
    EntityID highest_entity_under_cursor;
    u32 selection_limit;
    u32 prev_played_cards_count;
    u32 selection_count;
    Card_Number declared_number;
    EntityID entity_id_from_card_type[Category_Count][Card_Number_Count];
    f32 game_id_copie_message_time;
};

#endif //GAME_MAIN_H
