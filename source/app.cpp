#include "glad/glad.h" 
#include "glad/glad.c"
#include "GL/wglext.h"

#include "language_layer.h"
#include "maths.h"
#include "memory.h"
#include "strings.h"
#include "perlin.h"
#include "app.h"
#include "os.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "language_layer.c"
#include "memory.c"
#include "strings.c"
#include "perlin.c"
#include "os.c"
#include "shader.cpp"
#include "renderer.cpp"

// TODO(fakhri): i don't really like the way we currently do input hanlding probably we need something better
internal
void handle_events(Game_State *game_state)
{
    OS_Event *event = 0;
    
    while(OS_GetNextEvent(&event))
    {
        OS_EatEvent(event);
    }
}

extern "C"
{
    APP_PERMANENT_LOAD
    {
        os = os_;
        gladLoadGL();
        
        Game_State *game_state = os->game_state;
        *game_state = {};
        
        InitRenderer(&game_state->renderer);
    }
    
    APP_HOT_LOAD
    {
        os = os_;
        gladLoadGL();
    }
    
    APP_HOT_UNLOAD {
    }
    
    // BRAINSTORM(fakhri): the game should be a multiplayer game, each game sessoin is played between 4
    // players, players join a game session with their 3 friends (they should have their ip addresses
    // or something, I don't think we gonna have like some server for now that is going to be
    // responsible for assigning game sessions to players, we won't allow voice communication now, we 
    // will only have two action buttons, one to put the card and trust what the other player said
    // and the other to call the last player liar
    
    APP_UPDATE
    {
        Game_State *game_state = os->game_state;
        f32 dtime = os->dtime;
        
#if 0        
        if (game_state->did_join_a_game)
        {
            if (game_state->did_split_deck)
            {
                // NOTE(fakhri): great, we can play normally
                if (game_state->current_player_index == game_state->player_index)
                {
                }
                else
                {
                    u32 previous_player_index = (game_state->current_player_index - 1 + PLAYERS_COUNT) % PLAYERS_COUNT;
                    // NOTE(fakhri): was I the previous player?
                    if (game_state->player_index == previous_player_index)
                    {
                        // TODO(fakhri): see if the current player believed what I said
                        b32 did_current_player_believed = 0;
                        if (!did_current_player_believed && !game_state->did_tell_truth_last_play)
                        {
                            // NOTE(fakhri): ops, got to take all the cards on table
                        }
                        else
                        {
                        }
                    }
                }
            }
            else
            {
                // TODO(fakhri): split the deck
                if (game_state->is_hand_dealer)
                {
                    // TODO(fakhri): shuffle deck
                    // TODO(fakhri): split the deck between the players
                    // NOTE(fakhri): for now we will allow the player who deal the hand to kinda knows what each 
                    //other player hand is
                }
            }
        }
        else
        {
            if (game_state->attempting_to_join_a_game)
            {// TODO(fakhri): see if we joined a session
                game_state->joining_game_timeout -= dtime;
                if (game_state->joining_game_timeout <= 0)
                {
                    // NOTE(fakhri): we couldn't join
                    game_state->attempting_to_join_a_game = 0;
                }
            }
            else
            {
                // TODO(fakhri): attempt to join a session
            }
        }
#endif
        
        v2 pos = {0, 0};
        v2 size = {100, 100};
        
        glClearColor(0.1f, 0.1f, 0.1f, 1.f); 
        glClear(GL_COLOR_BUFFER_BIT);
        
        DebugDrawTexture(&game_state->renderer, game_state->renderer.test_2d_texture, pos, size);
        
    }
}