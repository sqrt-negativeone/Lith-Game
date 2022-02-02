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
    Input confirm;
    Input deceit;
    Input escape_key;
};

enum MessagesType
{
    // NOTE(fakhri): the client sends these message to the server to tell it about events
    Message_PlayCard, // NOTE(fakhri): needs to know what cards is played
    Message_QuestionCredibility, // NOTE(fakhri): doesn't need any extra data
    Message_BurnCards, // NOTE(fakhri): the burnt cards numbers
    
    // NOTE(fakhri): the server sends these messages to us so we can synchronize our state
    Message_ChangeTurn, // NOTE(fakhri): needs the player index who is going to play this turn
    Message_TakesCardsFromTable, // NOTE(fakhri): needs the inex of the player that will take the cards, and the numbers on these cards
    Message_PlayerBurntCards,
};

enum Game_Mode
{
    Game_Mode_BEGIN_MENU = 0,
    Game_Mode_MENU_JOIN_GAME,
    Game_Mode_MENU_MAIN,
    Game_Mode_MENU_HOST_GAME,
    Game_Mode_MENU_WAITING_PLAYERS,
    Game_Mode_END_MENU,
    Game_Mode_GAME,
};

struct Game_Session
{
    b32 game_started;
    u32 index_of_current_player;
    b32 is_hosting_game;
    b32 attempt_joining_session;
    s8 joined_players_usernames[PLAYERS_COUNT];
    u32 players_joined_sofar;
};

struct Game_State
{
    Game_Mode game_mode;
    
    u32 my_index;
    u32 selected_card_index;
    u32 hand_cards[DECK_CARDS_COUNT];
    u32 hand_cards_count[PLAYERS_COUNT];
    // NOTE(fakhri): only the server knows what's on theme
    u32 cards_on_table[DECK_CARDS_COUNT];
    u32 cards_on_table_count;
    u32 burnt_cards[DECK_CARDS_COUNT];
    u32 burnt_cards_count;
    
    // NOTE(fakhri): the card number i am suposed to play
    u32 card_number_to_play;
    
    b32 choosing_a_move;
    b32 is_table_empty;
    b32 should_claim_number;
    b32 played_my_turn;
    
    UI_Context ui_context;
    Rendering_Context rendering_context;
    Game_Session game_session;
};

struct Compile_Shader_Result
{
    b32 is_valid;
    GLuint program;
};

#endif //APP_H
