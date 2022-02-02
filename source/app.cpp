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
#include "buffer.cpp"
#include "ui.cpp"


internal
void PostNetworkMessage(MessagesType message_type)
{
    // TODO(fakhri): implement this
}

internal 
u32 ChangeSelectedItem(u32 item_index, i32 delta, u32 items_count)
{
    u32 result = (item_index + delta + items_count) % items_count;
    return result;
}

internal void
ChangeMenuSelectedItem(UI_Context *ui_context, i32 delta)
{
    Assert(ui_context);
    ui_context->last_time = os->time;
    ui_context->selected_item = ChangeSelectedItem(ui_context->selected_item, delta, ui_context->items_count);
}

internal
void PlayCard(Game_State *game_state, u32 claimed_value)
{
    // TODO(fakhri): implement this
    // TODO(fakhri): this function should tell the server about our move?
}


internal
void QuestionLastPlayerCredibility(Game_State *game_state)
{
    // TODO(fakhri): implement this
}

internal void
UI_ClearContext(UI_Context *ui_context)
{
    Assert(ui_context);
    ui_context->selected_item = 0;
    for (u32 field_buffer_index = 0;
         field_buffer_index < ArrayCount(ui_context->input_field_buffers);
         ++field_buffer_index)
    {
        EmptyBuffer(ui_context->input_field_buffers + field_buffer_index);
    }
}

internal void
OpenMenu(Game_State *game_state, Game_Mode menu_mode)
{
    Assert(Game_Mode_BEGIN_MENU < menu_mode && menu_mode < Game_Mode_END_MENU);
    UI_ClearContext(&game_state->ui_context);
    game_state->game_mode = menu_mode;
}

internal void
UpdateAndRenderGame(Game_State *game_state, Rendering_Context *rendering_context, Controller *controller)
{
    Game_Session *game_session = &game_state->game_session;
    f32 dtime = os->dtime;
    
    // NOTE(fakhri): see if the game started
    if (game_session->game_started)
    {
        // NOTE(fakhri): always allow player to change selected card
        if (controller->move_left.released)
        {
            controller->move_left.released = 0;
            u32 card_index = game_state->selected_card_index;
            game_state->selected_card_index = ChangeSelectedItem(card_index, -1, DECK_CARDS_COUNT);
        }
        if (controller->move_right.released)
        {
            controller->move_right.released = 0;
            u32 card_index = game_state->selected_card_index;
            game_state->selected_card_index = ChangeSelectedItem(card_index, +1, DECK_CARDS_COUNT);
        }
        
        // NOTE(fakhri): is it my turn ?
        if (game_session->index_of_current_player == game_state->my_index)
        {
            if (!game_state->played_my_turn && game_state->choosing_a_move)
            {
                if (controller->confirm.pressed)
                {
                    if (game_state->is_table_empty)
                    {
                        // NOTE(fakhri): prompt player to claim a number
                        game_state->should_claim_number = 1;
                    }
                    else
                    {
                        PlayCard(game_state, game_state->card_number_to_play);
                    }
                    game_state->choosing_a_move = 0;
                }
                if (controller->deceit.pressed)
                {
                    game_state->choosing_a_move = 0;
                    QuestionLastPlayerCredibility(game_state);
                }
            }
        }
        else
        {
            // NOTE(fakhri): it's not my turn, other players probably played something?
            // TODO(fakhri): did the other player finished his turn?
            // TODO(fakhri): is it now my turn ?
            // TODO(fakhri): should I take the cards in the pool? if yes what are they?
            // TODO(fakhri): did any cards got burnt? if yes which ones? and by any player?
        }
    }
    else
    {
        // TODO(fakhri): did the game started yet? if yes what are the cards in my hand? and who is
        // the  current player
    }
    
    // TODO(fakhri): draw the background
    glClearColor(0.1f, 0.1f, 0.1f, 1.f); 
    glClear(GL_COLOR_BUFFER_BIT);
    
    // TODO(fakhri): do this properly
    // NOTE(fakhri): draw the cards of players clockwise
    // player 0 to the left, 1 to the up, 2 right, and 3 down
    for (u32 player_index = 0;
         player_index < PLAYERS_COUNT;
         ++player_index)
    {
        v2 card_size = vec2(50, 100);
        v2 card_position = {};
        
        if (player_index == 0)
        {
            card_position = vec2(-0.9f * 0.5f *rendering_context->screen.width, 0.6f * 0.5f * rendering_context->screen.height);
        }
        if (player_index == 1)
        {
            card_position = vec2(-0.4f * 0.5f * rendering_context->screen.width, 0.5f * card_size.height + 0.5f * 0.5f * rendering_context->screen.height);
        }
        else if (player_index == 2)
        {
            card_position = vec2(0.9f * 0.5f * rendering_context->screen.width - card_size.width, 0.6f * 0.5f * rendering_context->screen.height);
        }
        else if (player_index == 3)
        {
            card_position = vec2(-0.4f * 0.5f * rendering_context->screen.width, -0.5f * 0.5f * rendering_context->screen.height);
        }
        f32 rotate = 0.f;
        if (player_index % 2 == 0)
        {
            rotate = PI / 2;
        }
        
        for (u32 my_card_index = 0;
             my_card_index < game_state->hand_cards_count[player_index];
             ++my_card_index)
        {
            v3 card_color = vec3(1.0f, 1.0f, 1.0f);
            if (player_index == game_state->my_index &&
                my_card_index == game_state->selected_card_index)
            {
                DebugDrawQuad(rendering_context, card_position, 1.15f * card_size, v3{1.f, 0.f, 0.f}, rotate);
            }
            
            DebugDrawQuad(rendering_context, card_position, card_size, card_color, rotate);
            if (player_index % 2)
            {
                // NOTE(fakhri): odd index player drawn horizonatly
                card_position.x += card_size.width + 10;
            }
            else
            {
                // NOTE(fakhri): even index drawn vertically
                card_position.y -= card_size.width + 10;
            }
        }
    }
    
    if (game_state->should_claim_number)
    {
        u32 claimed_value = 0;
        // TODO(fakhri): prompt the player with a menu to choose a number from to claim his card is
        v2 menu_size = 0.55f * rendering_context->screen;
        DebugDrawQuad(rendering_context, vec2(0, 0), menu_size, vec3(0.4f, 0.4f, 0.5f));
        
        if (controller->escape_key.released)
        {
            controller->escape_key.released = 0;
            game_state->should_claim_number = 0;
            game_state->choosing_a_move = 1;
        }
        
        if (claimed_value)
        {
            // NOTE(fakhri): play the turn
            PlayCard(game_state, claimed_value);
            game_state->played_my_turn = 1;
            game_state->should_claim_number = 0;
        }
    }
}

void UpdateAndRenderMainMenu(Game_State *game_state, Rendering_Context *rendering_context, UI_Context *ui_context, Controller *controller)
{
    v2 half_screen = 0.5f * rendering_context->screen;
    v2 screen = rendering_context->screen;
    
    UI_BeginFrame(ui_context);
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.f); 
    glClear(GL_COLOR_BUFFER_BIT);
    
    v2 item_pos = vec2(half_screen.x, 0.1f * screen.height);
    
    // NOTE(fakhri): render menu title
    // TODO(fakhri): use title fonts
    ChangeActiveFont(rendering_context, &rendering_context->arial_font);
    MenuItemLabel(game_state, S8Lit("Truth Or Lies?"), item_pos);
    
    // TODO(fakhri): use item fonts
    ChangeActiveFont(rendering_context, &rendering_context->arial_font);
    item_pos.y = 0.4f * screen.height;
    // NOTE(fakhri): join session button
    if (MenuItemButton(game_state, S8Lit("Join Game"), item_pos))
    {
        // NOTE(fakhri): join session was selected
        OpenMenu(game_state, Game_Mode_MENU_JOIN_GAME);
    }
    
    item_pos.y = 0.6f * screen.height;
    
    // NOTE(fakhri): host session button
    if (MenuItemButton(game_state, S8Lit("Host Game"), item_pos))
    {
        // NOTE(fakhri): host session was selected
        OpenMenu(game_state, Game_Mode_MENU_WAITING_PLAYERS);
        // TODO(fakhri): create a server thread
    }
    
    if (controller->move_down.pressed)
    {
        controller->move_down.pressed = 0;
        ChangeMenuSelectedItem(ui_context, -1);
    }
    if (controller->move_up.pressed)
    {
        controller->move_up.pressed = 0;
        ChangeMenuSelectedItem(ui_context, +1);
    }
}

void UpdateAndRenderWaitingPlayersMenu(Game_State *game_state,  Rendering_Context *rendering_context, UI_Context *ui_context, Controller *controller)
{
    
    v2 half_screen = 0.5f * rendering_context->screen;
    v2 screen = rendering_context->screen;
    
    UI_BeginFrame(ui_context);
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.f); 
    glClear(GL_COLOR_BUFFER_BIT);
    
    v3 white = vec3(1.0f, 1.0f, 1.0f);
    v3 none_white = 0.3f * white;
    
    
    // NOTE(fakhri): render menu title
    // TODO(fakhri): use title fonts
    ChangeActiveFont(rendering_context, &rendering_context->arial_font);
    v2 item_pos = vec2(half_screen.x, 0.05f * screen.height);
    MenuItemLabel(game_state, S8Lit("Joined Players"), item_pos);
    
    
    if (1 || game_state->game_session.is_hosting_game)
    {
        item_pos = vec2(half_screen.x, 0.12f * screen.height);
        MenuItemLabel(game_state, S8Lit("You are hosting the game"), item_pos);
    }
    
    
    item_pos.y = 0.9f * screen.height;
    
    MenuItemLabel(game_state, S8Lit("Waiting Players to Join"), item_pos, true);
    
    // TODO(fakhri): change font
    ChangeActiveFont(rendering_context, &rendering_context->arial_font);
    v2 background_size = 0.6f * screen;
    DebugDrawQuadScreenCoord(rendering_context, half_screen, background_size, 0.2f * white);
    
    // NOTE(fakhri): draw usernames of the connected players 
    // TODO(fakhri): use usernames font
    ChangeActiveFont(rendering_context, &rendering_context->arial_font);
    
#define PERCENTAGE 0.35f
    v2 coef[4] = {
        v2{    PERCENTAGE,     PERCENTAGE},
        v2{1 - PERCENTAGE,     PERCENTAGE},
        v2{    PERCENTAGE, 1 - PERCENTAGE},
        v2{1 - PERCENTAGE, 1 - PERCENTAGE},
    };
    
    for (u32 players_index = 0;
         players_index < game_state->game_session.players_joined_sofar;
         ++players_index)
    {
        item_pos = vec2(screen.x * coef[players_index].x,
                        screen.y * coef[players_index].y); 
        MenuItemLabel(game_state, S8Lit("user_name"), item_pos);
    }
    
    if (game_state->game_session.players_joined_sofar == PLAYERS_COUNT)
    {
        // NOTE(fakhri): enough players here
        game_state->game_mode = Game_Mode_GAME;
    }
}

void UpdateAndRenderJoinSessionMenu(Game_State *game_state,  Rendering_Context *rendering_context, UI_Context *ui_context, Controller *controller)
{
    
    UI_BeginFrame(ui_context);
    
    v2 half_screen = 0.5f * rendering_context->screen;
    v2 screen = rendering_context->screen;
    
    v3 white = vec3(1.0f, 1.0f, 1.0f);
    v3 none_white = 0.3f * white;
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.f); 
    glClear(GL_COLOR_BUFFER_BIT);
    
    ChangeActiveFont(rendering_context, &rendering_context->arial_font);
    v2 item_pos = vec2(half_screen.x, 0.05f * screen.height);
    MenuItemLabel(game_state, S8Lit("Joining a game"), item_pos);
    
    item_pos.y = 0.39f * screen.height;
    MenuItemLabel(game_state, S8Lit("host address"), item_pos);
    v2 item_size = {600, 50};
    item_pos.y = 0.5f * screen.height;
    MenuItemInputField(game_state, item_pos, item_size);
    
    item_pos.y = 0.7f * screen.height;
    if (!game_state->game_session.attempt_joining_session)
    {
        if (MenuItemButton(game_state, S8Lit("Join"), item_pos))
        {
            game_state->game_session.attempt_joining_session = 1;
            // TODO(fakhri): try to connect to the server
        }
        
        if (controller->escape_key.pressed)
        {
            controller->escape_key.pressed = 0;
            OpenMenu(game_state, Game_Mode_MENU_MAIN);
        }
    }
    else
    {
        f32 change = square_f(cos_f(0.5f * PI * os->time));
        v3 text_color = (0.4f  + 0.6f * change) * white;
        MenuItemLabel(game_state, S8Lit("Joining Game"), item_pos);
    }
    
    // NOTE(fakhri): process input
    if (controller->move_down.pressed)
    {
        controller->move_down.pressed = 0;
        ChangeMenuSelectedItem(ui_context, -1);
    }
    if (controller->move_up.pressed)
    {
        controller->move_up.pressed = 0;
        ChangeMenuSelectedItem(ui_context, +1);
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
        
        InitRenderer(&game_state->rendering_context);
        
        UI_Init(&game_state->ui_context, &os->permanent_arena);
        
        // TODO(fakhri): this is just for debug
        game_state->game_session = {
            1,
            3,
        };
        game_state->choosing_a_move = 1;
        game_state->game_mode = Game_Mode_MENU_MAIN;
        
        for (u32 player_index = 0;
             player_index < PLAYERS_COUNT;
             ++player_index)
        {
            game_state->hand_cards_count[player_index] = 10;
        }
        game_state->my_index = 3;
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
        Rendering_Context *rendering_context = &game_state->rendering_context;
        UI_Context *ui_context = &game_state->ui_context;
        Controller *controller = &os->controller;
        
        switch(game_state->game_mode)
        {
            case Game_Mode_GAME:
            {
                UpdateAndRenderGame(game_state, rendering_context, controller);
            } break;
            case Game_Mode_MENU_MAIN:
            {
                UpdateAndRenderMainMenu(game_state, rendering_context, ui_context, controller);
            } break;
            case Game_Mode_MENU_WAITING_PLAYERS: 
            {
                UpdateAndRenderWaitingPlayersMenu(game_state, rendering_context, ui_context, controller);
            } break;
            case Game_Mode_MENU_JOIN_GAME:
            {
                UpdateAndRenderJoinSessionMenu(game_state, rendering_context, ui_context, controller); 
            } break;
        }
        // NOTE(fakhri): ignore any input i didn't handle this frame
        OS_ClearEvents();
    }
}