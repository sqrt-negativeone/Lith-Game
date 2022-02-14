#include "glad/glad.h" 
#include "glad/glad.c"
#include "GL/wglext.h"

#include "language_layer.h"
#include "maths.h"
#include "memory.h"
#include "strings.h"
#include "perlin.h"
#include "network_message.h"
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
#include "network_message.cpp"

internal void
HandleAvailableNetworkMessages(Game_State *game_state)
{
    Game_Session *game_session = &game_state->game_session;
    NetworkMessage message = {};
    os->GetNextNetworkMessageIfAvailable(&message);
    // TODO(fakhri): we can limit the nombre of messages handled per frame if it slowed things down
    for(;
        message.is_available;
        os->GetNextNetworkMessageIfAvailable(&message)
        )
    {
        switch(message.type)
        {
            case NetworkMessageType_From_Server_PLAYER_TURN:
            {
                // TODO(fakhri): change the turn
            } break;
            case NetworkMessageType_From_Server_PLAYER_JOINED_GAME:
            {
                u32 player_index = message.player_index;
                game_session->players[player_index] = message.player_username;
                ++game_session->players_joined_sofar;
            } break;
            case NetworkMessageType_From_Server_INVALIDE_USERNAME:
            {
            } break;
            default:
            {
                // NOTE(fakhri): unhandled message
                BreakDebugger();
            } break;
        }
    }
}

inline void
OpenMenu(Game_State *game_state, Game_Mode menu_mode)
{
    Assert(Game_Mode_MENU_BEGIN < menu_mode && menu_mode < Game_Mode_MENU_END);
    UI_ClearContext(&game_state->ui_context);
    game_state->game_mode = menu_mode;
}

internal void
MoveEntity(Game_State *game_state, Entity *entity, f32 spring_constant, f32 friction, f32 mass)
{
    if (entity->followed_entity_index)
    {
        Entity *followed_entity = game_state->entities + entity->followed_entity_index;
        f32 following_trigger_distance = .1f;
        if (LengthSquaredVec2(followed_entity->center_pos - entity->center_pos) > following_trigger_distance)
        {
            entity->target_pos = followed_entity->center_pos;
        }
    }
    
    v2 acceleration = -spring_constant * (entity->center_pos - entity->target_pos) - friction * entity->velocity;
    acceleration *= 1.0f / mass;
    entity->velocity += os->dtime * acceleration;
    entity->center_pos += os->dtime * entity->velocity + 0.5f * square_f(os->dtime) * acceleration;
}

internal void
UpdateCursorEntity(Game_State *game_state, Entity *entity)
{
    entity->center_pos = ScreenToWorldCoord(&game_state->rendering_context, os->mouse_position);
}

internal void
UpdateCardEntity(Game_State *game_state, Entity *entity)
{
    Entity *cursor_entity = game_state->entities + (u32)Entity_Type_Cursor_Entity;
    Assert(cursor_entity->type == Entity_Type_Cursor_Entity);
    
    b32 should_be_under_cursor = IsInsideRect(RectCentDim(entity->center_pos, entity->current_dimension), cursor_entity->center_pos);
    
    if (!entity->is_under_cursor && should_be_under_cursor)
    {
        entity->is_under_cursor = true;
        entity->target_dimension = 1.1f * entity->original_dimension;
    }
    
    if (entity->is_under_cursor && !should_be_under_cursor)
    {
        entity->is_under_cursor = false;
        entity->target_dimension = entity->original_dimension;
    }
    
    if (entity->is_under_cursor)
    {
        if (!entity->is_pressed && os->controller.left_mouse.pressed)
        {
            entity->is_pressed = true;
            entity->followed_entity_index = Entity_Type_Cursor_Entity;
        }
    }
    
    if (entity->is_pressed)
    {
        if (os->controller.left_mouse.released)
        {
            entity->is_pressed = false;
            entity->target_pos = entity->original_pos;
            entity->followed_entity_index = 0;
        }
    }
    
    // NOTE(fakhri): update dimension
    entity->current_dimension = Vec2MoveTowards(entity->current_dimension,
                                                entity->target_dimension,
                                                os->dtime * entity->dDimension);
    
    MoveEntity(game_state, entity, 9000, 600, 10);
}

internal void
UpdateCardNumberEntity(Game_State *game_state, Entity *entity)
{
    Assert(entity->followed_entity_index);
    
    MoveEntity(game_state, entity, 10000.f, 150.f, 10.f);
    Entity *followed_entity = game_state->entities + entity->followed_entity_index;
    entity->center_pos = ClampInsideRect(RectCentDim(followed_entity->center_pos, followed_entity->current_dimension), entity->center_pos);
}

internal void
ChangeResidency(Entity *entity, Card_Residency residency)
{
    Assert(entity);
    if (entity->residency != residency)
    {
        
    }
}

// TODO(fakhri): make the coordinates pixel independents
// TODO(fakhri): come up with a client-server protocol for the game
internal void
UpdateAndRenderGame(Game_State *game_state, Rendering_Context *rendering_context, Controller *controller)
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.f); 
    glClear(GL_COLOR_BUFFER_BIT);
    v3 white = vec3(1, 1, 1);
    v3 red = vec3(1, 0, 0);
    
    // NOTE(fakhri): loop over all the entities and update them
    for (u32 entity_index = 1;
         entity_index < game_state->entity_count;
         ++entity_index)
    {
        Entity *entity = game_state->entities + entity_index;
        switch(entity->type)
        {
            case Entity_Type_Cursor_Entity:
            {
                UpdateCursorEntity(game_state, entity);
                // TODO(fakhri): render the cursor here?
            } break;
            case Entity_Type_Entity_Card:
            {
                UpdateCardEntity(game_state, entity);
                if (entity->is_pressed)
                {
                    DebugDrawQuadWorldCoord(rendering_context, entity->center_pos, 1.05f * entity->current_dimension, red);
                }
                DebugDrawQuadWorldCoord(rendering_context, entity->center_pos, entity->current_dimension, white);
            } break;
            case Entity_Type_Entity_Card_Number:
            {
                UpdateCardNumberEntity(game_state, entity);
                char text[2] = {'0' + (char)entity->card_number,};
                s8 number = S8Lit(text);
                ChangeActiveFont(rendering_context, &rendering_context->arial_font);
                DebugDrawTextWorldCoord(rendering_context, number, entity->center_pos, red);
            } break;
            default:
            {
                BreakDebugger();
            } break;
        }
    }
    
}

internal
void UpdateAndRenderMainMenu(Game_State *game_state, Rendering_Context *rendering_context, UI_Context *ui_context, Controller *controller)
{
    Game_Session *game_session = &game_state->game_session;
    v2 half_screen = 0.5f * rendering_context->screen;
    v2 screen = rendering_context->screen;
    
    UI_BeginFrame(ui_context);
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.f); 
    glClear(GL_COLOR_BUFFER_BIT);
    
    UI_PositionItem(ui_context, vec2(half_screen.x, 0.1f * screen.height));
    UI_SetVerticalSpacing(ui_context, 0.3f * screen.y);
    
    // NOTE(fakhri): render menu title
    // TODO(fakhri): use title fonts
    ChangeActiveFont(rendering_context, &rendering_context->arial_font);
    UI_MenuItemLabel(game_state, S8Lit("Truth Or Lies?"));
    
    // TODO(fakhri): use item fonts
    ChangeActiveFont(rendering_context, &rendering_context->arial_font);
    UI_VerticalAdvanceItemPosition(ui_context);
    
    UI_SetVerticalSpacing(ui_context, 0.15f * screen.y);
    
    // NOTE(fakhri): join session button
    if (UI_MenuItemButton(game_state, S8Lit("Join Game Room")))
    {
        // NOTE(fakhri): join session was clicked
        OpenMenu(game_state, Game_Mode_MENU_JOIN_GAME);
    }
    
    UI_VerticalAdvanceItemPosition(ui_context);
    // NOTE(fakhri): host session button
    if (UI_MenuItemButton(game_state, S8Lit("Create Game Room")))
    {
        // NOTE(fakhri): host session was clicked
        OpenMenu(game_state, Game_Mode_MENU_WAITING_PLAYERS);
        os->PushNetworkMessage(CreateNewGameSessionMessage());
        SetFlag(game_session->session_state_flags, SESSION_FLAG_HOSTING_GAME);
    }
    
    UI_EndFrame(ui_context, controller);
}

void UpdateAndRenderJoinSessionMenu(Game_State *game_state,  Rendering_Context *rendering_context, UI_Context *ui_context, Controller *controller)
{
    Game_Session *game_session = &game_state->game_session;
    
    UI_BeginFrame(ui_context);
    v2 half_screen = 0.5f * rendering_context->screen;
    v2 screen = rendering_context->screen;
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.f); 
    glClear(GL_COLOR_BUFFER_BIT);
    
    ChangeActiveFont(rendering_context, &rendering_context->arial_font);
    UI_PositionItem(ui_context, vec2(half_screen.x, 0.05f * screen.height));
    
    UI_SetVerticalSpacing(ui_context, 0.2f * screen.y);
    
    UI_MenuItemLabel(game_state, S8Lit("Connect to game"));
    
    u32 host_address_index;
    
    // NOTE(fakhri): host address input field
    {
        UI_VerticalAdvanceItemPosition(ui_context);
        UI_MenuItemLabel(game_state, S8Lit("host address"));
        v2 item_size = {600, 50};
        host_address_index = ui_context->items_count;
        UI_VerticalAdvanceItemPosition(ui_context);
        UI_MenuItemInputField(game_state, item_size);
    }
    
    UI_VerticalAdvanceItemPosition(ui_context);
    
    if (!IsFlagSet(game_session->session_state_flags, SESSION_FLAG_TRYING_CONNECT_GAME))
    {
        // TODO(fakhri): just for debug while we don't have a working server
        SetFlag(game_session->session_state_flags, SESSION_FLAG_CONNECTED_TO_GAME);
        
        if (UI_MenuItemButton(game_state, S8Lit("Connect")))
        {
            SetFlag(game_session->session_state_flags, SESSION_FLAG_TRYING_CONNECT_GAME);
            s8 input =  ui_context->input_field_buffers[host_address_index].buffer;
            os->PushNetworkMessage(CreateConnectToServerMessage(&os->permanent_arena, input));
        }
        if (controller->escape_key.pressed)
        {
            OpenMenu(game_state, Game_Mode_MENU_MAIN);
        }
    }
    else
    {
        // NOTE(fakhri): trying to connect to game
        UI_MenuItemLabel(game_state, S8Lit("Connecting to Game"));
        if (IsFlagSet(game_session->session_state_flags, SESSION_FLAG_CONNECTED_TO_GAME))
        {
            // NOTE(fakhri): good, we should now enter our username and see if it's valid
            OpenMenu(game_state, Game_Mode_MENU_USERNAME);
            ClearFlag(game_session->session_state_flags, SESSION_FLAG_TRYING_CONNECT_GAME);
        }
        else if (IsFlagSet(game_session->session_state_flags, SESSION_FLAG_FAILED_CONNECT_GAME))
        {
            // TODO(fakhri): show some message indicating that we couldn't connect to server
        }
    }
    
    UI_EndFrame(ui_context, controller);
}

void UpdateAndRenderUserNameMenu(Game_State *game_state,  Rendering_Context *rendering_context, UI_Context *ui_context, Controller *controller)
{
    Game_Session *game_session = &game_state->game_session;
    UI_BeginFrame(ui_context);
    
    v2 half_screen = 0.5f * rendering_context->screen;
    v2 screen = rendering_context->screen;
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.f); 
    glClear(GL_COLOR_BUFFER_BIT);
    
    ChangeActiveFont(rendering_context, &rendering_context->arial_font);
    
    UI_PositionItem(ui_context, vec2(half_screen.x, 0.3f * screen.height));
    UI_MenuItemLabel(game_state, S8Lit("Please Enter a username"));
    
    u32 username_index = ui_context->items_count;
    
    v2 item_size = {600, 50};
    UI_VerticalAdvanceItemPosition(ui_context);
    UI_MenuItemInputField(game_state, item_size);
    
    UI_VerticalAdvanceItemPosition(ui_context);
    if (!IsFlagSet(game_session->session_state_flags, SESSION_FLAG_TRYING_JOIN_GAME))
    {
        // TODO(fakhri): just for debug while we don't have a working server
        SetFlag(game_session->session_state_flags, SESSION_FLAG_JOINED_GAME);
        
        if (UI_MenuItemButton(game_state, S8Lit("Join")))
        {
            s8 username =  ui_context->input_field_buffers[username_index].buffer;
            
            os->PushNetworkMessage(CreateUsernameRequest(username));
            // TODO(fakhri): make sure the username isn't empty
            SetFlag(game_session->session_state_flags, SESSION_FLAG_TRYING_JOIN_GAME);
        }
    }
    else
    {
        if (IsFlagSet(game_session->session_state_flags, SESSION_FLAG_JOINED_GAME))
        {
            // NOTE(fakhri): good, we wait for all players to join and then we start the game
            OpenMenu(game_state, Game_Mode_MENU_WAITING_PLAYERS);
            ClearFlag(game_session->session_state_flags, SESSION_FLAG_TRYING_JOIN_GAME);
        }
        if (IsFlagSet(game_session->session_state_flags, SESSION_FLAG_FAILED_JOIN_GAME))
        {
            // TODO(fakhri): display message indicating that we couldn't join the game and the reason
        }
    }
    UI_EndFrame(ui_context, controller);
}

internal
void UpdateAndRenderWaitingPlayersMenu(Game_State *game_state,  Rendering_Context *rendering_context, UI_Context *ui_context, Controller *controller)
{
    Game_Session *game_session = &game_state->game_session;
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
    
    UI_PositionItem(ui_context, vec2(half_screen.x, 0.05f * screen.height));
    UI_SetVerticalSpacing(ui_context, 0.2f * screen.y);
    UI_MenuItemLabel(game_state, S8Lit("Joined Players"));
    
    // TODO(fakhri): remove the 1 from here
    if (IsFlagSet(game_session->session_state_flags, SESSION_FLAG_HOSTING_GAME))
    {
        UI_VerticalAdvanceItemPosition(ui_context);
        UI_MenuItemLabel(game_state, S8Lit("You are hosting the game"));
        
        // TODO(fakhri): format this text
        UI_VerticalAdvanceItemPosition(ui_context);
        s8 server_addres_message = S8Lit("the game session address is 127.0.0.1");
        UI_MenuItemLabel(game_state, server_addres_message);
        
        UI_VerticalAdvanceItemPosition(ui_context);
        if (UI_MenuItemButton(game_state, S8Lit("Copy to Clipboard")))
        {
            // TODO(fakhri): copy server address to clipboard
        }
    }
    
    // TODO(fakhri): change font
    ChangeActiveFont(rendering_context, &rendering_context->arial_font);
    
    // NOTE(fakhri): draw usernames of the connected players 
    // TODO(fakhri): use usernames font
    ChangeActiveFont(rendering_context, &rendering_context->arial_font);
    
    UI_VerticalAdvanceItemPosition(ui_context);
    UI_SetVerticalSpacing(ui_context, 0.1f * screen.height);
    for (u32 players_index = 0;
         players_index < PLAYERS_COUNT;
         ++players_index)
    {
        UI_VerticalAdvanceItemPosition(ui_context);
        UI_MenuItemLabel(game_state, game_session->players[players_index]);
    }
    
    UI_SetVerticalSpacing(ui_context, 0.2f * screen.height);
    UI_VerticalAdvanceItemPosition(ui_context);
    UI_MenuItemLabel(game_state, S8Lit("Waiting Players to Join"), true);
    
    if (game_session->players_joined_sofar == PLAYERS_COUNT)
    {
        // NOTE(fakhri): enough players here
        game_state->game_mode = Game_Mode_GAME;
    }
}

internal u32
AddEntity(Game_State *game_state)
{
    Assert(game_state->entity_count < ArrayCount(game_state->entities));
    u32 entity_index = game_state->entity_count++;
    return entity_index;
}

internal void
AddNullEntity(Game_State *game_state)
{
    u32 entity_index = AddEntity(game_state);
    Entity *entity = game_state->entities + entity_index;
    entity->type = Entity_Type_Null_Entity;
}

internal void
AddCursorEntity(Game_State *game_state)
{
    u32 entity_index = AddEntity(game_state);
    Entity *entity = game_state->entities + entity_index;
    entity->type = Entity_Type_Cursor_Entity;
}

internal void
AddCardNumberEntity(Game_State *game_state, u32 card_number, u32 card_entity_index)
{
    u32 number_entity_index = AddEntity(game_state);
    Entity *number_entity = game_state->entities + number_entity_index;
    Entity *card_entitiy = game_state->entities + card_entity_index;
    *number_entity = {};
    number_entity->type = Entity_Type_Entity_Card_Number;
    number_entity->center_pos = card_entitiy->center_pos;
    number_entity->target_pos = card_entitiy->center_pos;
    number_entity->current_dimension = vec2(0.5f, 0.5f);
    number_entity->card_number = card_number;
    number_entity->followed_entity_index = card_entity_index;
}

internal void
AddCardEntity(Game_State *game_state, v2 starting_pos, u32 card_number)
{
    u32 card_entity_index = AddEntity(game_state);
    Entity *card = game_state->entities + card_entity_index;
    *card = {};
    card->type = Entity_Type_Entity_Card;
    card->original_pos = starting_pos;
    card->center_pos = starting_pos;
    card->target_pos = starting_pos;
    
#if 0    
    card->original_dimension = vec2(90, 180);
    card->target_dimension   = vec2(90, 180);
    card->current_dimension  = vec2(90, 180);
#else
    card->original_dimension = vec2(8, 15);
    card->target_dimension   = card->original_dimension;
    card->current_dimension  = card->original_dimension;
#endif
    
    card->dDimension = 20.f;
    AddCardNumberEntity(game_state, card_number, card_entity_index);
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
        game_state->game_mode = Game_Mode_GAME;
        game_state->game_session = {};
        game_state->game_session.players_joined_sofar = PLAYERS_COUNT;
        game_state->game_session.players[0] = S8Lit("0");
        game_state->game_session.players[1] = S8Lit("1");
        game_state->game_session.players[2] = S8Lit("2");
        game_state->game_session.players[3] = S8Lit("3");
        
        // TODO(fakhri): add some bounce for the numbers when the cards moves, that should look cool lol
        AddNullEntity(game_state);
        AddCursorEntity(game_state);
        
        for (u32 card_index = 0;
             card_index < 1;
             ++card_index)
        {
            v2 card_pos = vec2(50.f, 20.f);
            AddCardEntity(game_state, card_pos, card_index);
        }
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
        
        HandleAvailableNetworkMessages(game_state);
        UpdateScreenSize(rendering_context);
        
        switch(game_state->game_mode)
        {
            case Game_Mode_GAME:
            {
                UpdateAndRenderGame(game_state, rendering_context, controller);
            } break;
            case Game_Mode_MENU_USERNAME:
            {
                UpdateAndRenderUserNameMenu(game_state, rendering_context, ui_context, controller);
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
        
        DebugDrawQuadScreenCoord(rendering_context, os->mouse_position, vec2(10, 10), vec3(1, .3f, .5f));
        
#if 0
        char buffer[64];
        sprintf(buffer, "frame time : %f", os->dtime);
        s8 frame_time = s8{buffer, CalculateCStringLength(buffer)};
        DebugDrawText(rendering_context, frame_time, vec2(400, 20), vec3(1,1,1));
        sprintf(buffer, "frame fps : %f", 1.0f / os->dtime);
        s8 frame_fps = s8{buffer, CalculateCStringLength(buffer)};
        DebugDrawText(rendering_context, frame_time, vec2(400, 60), vec3(1,1,1));
#endif
        
        // NOTE(fakhri): ignore any input i didn't handle this frame
        if (os->controller.toggle_fullscreen.pressed)
        {
            os->fullscreen ^= 1;
        }
        os->controller = {};
    }
}