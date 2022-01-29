/* date = January 15th 2022 1:05 am */

#ifndef APP_H
#define APP_H

#include "renderer.h"

#define PLAYERS_COUNT 4

struct Game_State
{
    Renderer renderer;
    
    b32 attempting_to_join_a_game;
    f32 joining_game_timeout;
    b32 did_join_a_game;
    
    u32 current_player_index;
    u32 player_index;
    b32 is_hand_dealer;
    
    b32 did_tell_truth_last_play;
    b32 did_split_deck;
};

struct Compile_Shader_Result
{
    b32 is_valid;
    GLuint program;
};

#endif //APP_H
