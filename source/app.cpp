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
ReorganizeResidencyCards(Game_State *game_state, Card_Residency residency_type)
{
    if (residency_type != Card_Residency_None)
    {
        f32 world_width = MAX_UNITS_PER_X;
        f32 world_height = MAX_UNITS_PER_X * game_state->rendering_context.aspect_ratio;
        
        Residency *entity_residency = game_state->entity_residencies + residency_type;
        u32 entity_count = entity_residency->entity_count;
        v3 start_point = {};
        b32 change_x = false;
        b32 change_y = false;
        b32 change_z = false;
        switch(residency_type)
        {
            case Card_Residency_Up:
            {
                change_x = true;
                start_point.x = 0.5f * (CARD_WIDTH + world_width - entity_count * CARD_WIDTH - (entity_count - 1) * CARD_X_GAP);
                start_point.y = world_height - 6.5f;
            } break;
            case Card_Residency_Down:
            {
                change_x = true;
                start_point.y = 6.5f;
                start_point.x = 0.5f * (CARD_WIDTH + world_width - entity_count * CARD_WIDTH - (entity_count - 1) * CARD_X_GAP);
            } break;
            case Card_Residency_Left:
            {
                change_y = true;
                start_point.x = 6.5f;
                start_point.y = 0.5f * (CARD_HEIGHT + world_height - entity_count * CARD_HEIGHT - (entity_count - 1) * CARD_Y_GAP);
            } break;
            case Card_Residency_Right:
            {
                change_y = true;
                start_point.x = world_width - 6.5f;
                start_point.y = 0.5f * (CARD_HEIGHT + world_height - entity_count * CARD_HEIGHT - (entity_count - 1) * CARD_Y_GAP);
            } break;
            case Card_Residency_Table:
            {
                start_point.xy = vec2(0.5f * world_width, 0.55f * world_height);
            } break;
#if 0
            case Card_Residency_Burnt:
            {
            } break;
#endif
            default:
            {
                // NOTE(fakhri): not yet implemented
                BreakDebugger();
            } break;
        }
        if (change_x)
        {
            for (u32 residency_index = 0;
                 residency_index < entity_residency->entity_count;
                 ++residency_index)
            {
                Entity *entity = game_state->entities + entity_residency->entity_indices[residency_index];
                entity->residency_pos = start_point;
                if (!entity->followed_entity_index)
                {
                    entity->target_pos = entity->residency_pos;
                }
                start_point.x += CARD_WIDTH + CARD_X_GAP;
            }
        }
        else if (change_y)
        {
            for (u32 residency_index = 0;
                 residency_index < entity_residency->entity_count;
                 ++residency_index)
            {
                Entity *entity = game_state->entities + entity_residency->entity_indices[residency_index];
                entity->residency_pos = start_point;
                if (!entity->followed_entity_index)
                {
                    entity->target_pos = entity->residency_pos;
                }
                start_point.y += CARD_HEIGHT + CARD_Y_GAP;
                start_point.z += 0.3f;
            }
        }
        else
        {
            for (u32 residency_index = 0;
                 residency_index < entity_residency->entity_count;
                 ++residency_index)
            {
                Entity *entity = game_state->entities + entity_residency->entity_indices[residency_index];
                entity->residency_pos = start_point;
                if (!entity->followed_entity_index)
                {
                    entity->target_pos = entity->residency_pos;
                }
                start_point.y += 0.1f;
                start_point.z += 0.3f;
            }
        }
    }
}

internal void
ChangeResidency(Game_State *game_state, u32 entity_index, Card_Residency residency)
{
    Entity *entity = game_state->entities + entity_index;
    Card_Residency old_residency = entity->residency;
    if (residency != Card_Residency_None && old_residency != residency)
    {
        if (old_residency != Card_Residency_None)
        {
            // NOTE(fakhri): remove the entity from the old residency
            Residency *entity_residency = game_state->entity_residencies + old_residency;
            for (u32 residency_index = 0;
                 residency_index < entity_residency->entity_count;
                 ++residency_index)
            {
                if (entity_residency->entity_indices[residency_index] == entity_index)
                {
                    for (; residency_index < entity_residency->entity_count; ++residency_index)
                    {
                        entity_residency->entity_indices[residency_index] = entity_residency->entity_indices[residency_index + 1];
                    }
                    --entity_residency->entity_count;
                    break;
                }
            }
        }
        
        // NOTE(fakhri): add entity to the new residency
        Residency *entity_residency = game_state->entity_residencies + residency;
        Assert(entity_residency->entity_count < ArrayCount(entity_residency->entity_indices));
        entity_residency->entity_indices[entity_residency->entity_count++] = entity_index;
        entity->residency = residency;
        
        ReorganizeResidencyCards(game_state, old_residency);
        ReorganizeResidencyCards(game_state, residency);
    }
}

internal void
MoveEntity(Game_State *game_state, Entity *entity, f32 spring_constant, f32 friction, f32 mass)
{
    if (entity->followed_entity_index)
    {
        Entity *followed_entity = game_state->entities + entity->followed_entity_index;
        f32 following_trigger_distance = 0.0f;
        if (LengthSquaredVec2(followed_entity->center_pos.xy - entity->center_pos.xy) > following_trigger_distance)
        {
            entity->target_pos.xy = followed_entity->center_pos.xy + entity->offset_in_follwed_entity;
        }
    }
    
    v2 acceleration = -spring_constant * (entity->center_pos.xy - entity->target_pos.xy) - friction * entity->velocity;
    acceleration *= 1.0f / mass;
    entity->velocity += os->dtime * acceleration;
    entity->center_pos.xy += os->dtime * entity->velocity + 0.5f * square_f(os->dtime) * acceleration;
    entity->center_pos.z = entity->target_pos.z;
}

internal void
UpdateCursorEntity(Game_State *game_state, Entity *entity)
{
    entity->center_pos.xy = ScreenToWorldCoord(&game_state->rendering_context, os->mouse_position);
}

internal void
UpdateCardEntity(Game_State *game_state, u32 entity_index)
{
    Entity *entity = game_state->entities + entity_index;
    Entity *cursor_entity = game_state->entities + (u32)Entity_Type_Cursor_Entity;
    Assert(cursor_entity->type == Entity_Type_Cursor_Entity);
    
    b32 should_be_under_cursor = IsInsideRect(RectCentDim(entity->center_pos.xy, entity->current_dimension), cursor_entity->center_pos.xy) ;
    
    if (!entity->is_under_cursor && should_be_under_cursor)
    {
        entity->is_under_cursor = true;
        entity->target_dimension = 1.1f * vec2(CARD_WIDTH, CARD_HEIGHT);
    }
    
    if (entity->is_under_cursor && !should_be_under_cursor)
    {
        entity->is_under_cursor = false;
        entity->target_dimension = vec2(CARD_WIDTH, CARD_HEIGHT);
    }
    
    if (entity->is_under_cursor)
    {
        if (!entity->is_pressed)
        {
            if(os->controller.left_mouse.pressed && entity->residency == Card_Residency_Down)
            {
                // NOTE(fakhri): make sure we are not pressing another card
                if (game_state->card_pressed_index == 0)
                {
                    entity->is_pressed = true;
                    entity->followed_entity_index = Entity_Type_Cursor_Entity;
                    game_state->card_pressed_index = entity_index;
                    entity->target_pos.z = 50.f;
                }
            }
        }
    }
    
    if (entity->is_pressed)
    {
        
        f32 world_width = MAX_UNITS_PER_X;
        f32 world_height = MAX_UNITS_PER_X * game_state->rendering_context.aspect_ratio;
        v2 world_dimension = vec2(world_width, world_height);
        
        v2 table_center = vec2(0.5f * world_width, 0.55f * world_height);
        b32 can_move_to_table = false;
        if (IsInsideRect(RectCentDim(table_center, vec2(20, 20)), entity->center_pos.xy))
        {
            
            ChangeActiveFont(&game_state->rendering_context, &game_state->rendering_context.arial_font);
            DebugDrawTextWorldCoord(&game_state->rendering_context, S8Lit("release your mouse to play the card"), 0.5f * world_dimension,vec3(1,1,1));
            can_move_to_table = 1;
        }
        
        if (os->controller.left_mouse.released)
        {
            entity->is_pressed = false;
            entity->target_pos = entity->residency_pos;
            entity->followed_entity_index = 0;
            game_state->card_pressed_index = 0;
            entity->target_pos.z =  entity->residency_pos.z;
            
            if (can_move_to_table)
            {
                entity->target_y_angle = PI;
                ChangeResidency(game_state, entity_index, Card_Residency_Table);
            }
        }
        
    }
    
    entity->y_angle = MoveTowards(entity->y_angle, entity->target_y_angle, entity->dy_angle * os->dtime);
    
    // NOTE(fakhri): update dimension
    entity->current_dimension = Vec2MoveTowards(entity->current_dimension,
                                                entity->target_dimension,
                                                os->dtime * entity->dDimension);
    
    MoveEntity(game_state, entity, 900, 60, 1);
}

internal void
UpdateCompanionEntity(Game_State *game_state, Entity *entity)
{
    Assert(entity->followed_entity_index);
    
    MoveEntity(game_state, entity, 800.f, 10.f, 1.f);
    Entity *followed_entity = game_state->entities + entity->followed_entity_index;
    entity->center_pos.xy = ClampInsideRect(RectCentDim(followed_entity->center_pos.xy, followed_entity->current_dimension), entity->center_pos.xy);
}

// TODO(fakhri): come up with a client-server protocol for the game
internal void
UpdateAndRenderGame(Game_State *game_state, Rendering_Context *rendering_context, Controller *controller)
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    v3 white = vec3(1, 1, 1);
    v3 red = vec3(1, 0, 0);
    
#if 1    
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
                UpdateCardEntity(game_state, entity_index);
                if (entity->is_pressed)
                {
                    DebugDrawQuadWorldCoord(rendering_context, vec3(entity->center_pos.xy, entity->center_pos.z - 0.01f), 1.05f * entity->current_dimension, red, entity->y_angle);
                }
                
                DebugDrawTextureWorldCoord(rendering_context, entity->texture, entity->center_pos, entity->current_dimension, entity->y_angle);
                
                v3 card_back_pos = entity->center_pos;
                DebugDrawTextureWorldCoord(rendering_context, game_state->rendering_context.frensh_deck.card_back_texture,
                                           card_back_pos, entity->current_dimension, PI - entity->y_angle);
                
            } break;
            case Entity_Type_Entity_Companion:
            {
                UpdateCompanionEntity(game_state, entity);
                Entity *followed_entity = game_state->entities + entity->followed_entity_index;
                Assert(followed_entity);
                DebugDrawTextureWorldCoord(rendering_context, entity->texture, vec3(entity->center_pos.xy, followed_entity->center_pos.z + 0.01f), entity->current_dimension, followed_entity->y_angle);
            } break;
            default:
            {
                BreakDebugger();
            } break;
        }
    }
#endif
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
AddCompanionEntity(Game_State *game_state, GLuint texture, v2 companion_dimensions, u32 followed_entity_index, v2 followed_offset)
{
    u32 companion_entity_index = AddEntity(game_state);
    Entity *companion_entity = game_state->entities + companion_entity_index;
    Entity *card_entitiy = game_state->entities + followed_entity_index;
    *companion_entity = {};
    companion_entity->type = Entity_Type_Entity_Companion;
    companion_entity->center_pos = card_entitiy->center_pos;
    companion_entity->target_pos = card_entitiy->center_pos;
    companion_entity->current_dimension = companion_dimensions;
    companion_entity->followed_entity_index = followed_entity_index;
    companion_entity->offset_in_follwed_entity = followed_offset;
    companion_entity->texture = texture;
}

internal inline void
AddCompanion_Ace(Game_State *game_state, GLuint category, v2 companion_size, u32 card_entity_index)
{
    AddCompanionEntity(game_state, category, companion_size, card_entity_index, vec2(0, 0));
}

internal inline void
AddCompanion_2(Game_State *game_state, GLuint category_up, GLuint category_down, v2 companion_size, v2 card_dimension, u32 card_entity_index)
{
    v2 padding = vec2(0.5f, 0.3f);
    v2 companion_offset = +0.5f * card_dimension - 0.5f * companion_size - padding;
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_index, 
                       vec2(0, companion_offset.y));
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_index, 
                       vec2(0, -companion_offset.y));
}

internal inline void
AddCompanion_3(Game_State *game_state, GLuint category_up, GLuint category_down, v2 companion_size, v2 card_dimension, u32 card_entity_index)
{
    AddCompanion_Ace(game_state, category_up, companion_size, card_entity_index);
    
    AddCompanion_2(game_state, category_up, category_down, companion_size, card_dimension, card_entity_index);
}

internal inline void
AddCompanion_4(Game_State *game_state, GLuint category_up, GLuint category_down, v2 companion_size, v2 card_dimension, u32 card_entity_index)
{
    v2 padding = vec2(0.5f, 0.3f);
    v2 companion_offset = +0.5f * card_dimension - 1.0f * companion_size - padding;
    
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_index, 
                       vec2(-companion_offset.x, companion_offset.y));
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_index, 
                       vec2(companion_offset.x, companion_offset.y));
    
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_index, 
                       vec2(-companion_offset.x, -companion_offset.y));
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_index, 
                       vec2(companion_offset.x, -companion_offset.y));
    
}

internal inline void
AddCompanion_5(Game_State *game_state, GLuint category_up, GLuint category_down, v2 companion_size, v2 card_dimension, u32 card_entity_index)
{
    AddCompanion_Ace(game_state, category_up, companion_size, card_entity_index);
    
    AddCompanion_4(game_state, category_up, category_down, companion_size, card_dimension, card_entity_index);
}

internal inline  void
AddCompanion_6(Game_State *game_state, GLuint category_up, GLuint category_down, v2 companion_size, v2 card_dimension, u32 card_entity_index)
{
    AddCompanion_4(game_state, category_up, category_down, companion_size, card_dimension, card_entity_index);
    
    v2 padding = vec2(0.5f, 0.3f);
    v2 companion_offset = +0.5f * card_dimension - 1.0f * companion_size - padding;
    
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_index, 
                       vec2(-companion_offset.x, 0));
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_index, 
                       vec2(companion_offset.x, 0));
    
}

internal inline void
AddCompanion_7(Game_State *game_state, GLuint category_up, GLuint category_down, v2 companion_size, v2 card_dimension, u32 card_entity_index)
{
    AddCompanion_6(game_state, category_up, category_down, companion_size, card_dimension, card_entity_index);
    
    v2 padding = vec2(0.5f, 0.3f);
    v2 companion_offset = +0.5f * card_dimension - 1.0f * companion_size - padding;
    
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_index, 
                       vec2(0, 0.5f * companion_offset.y));
    
}

internal inline void
AddCompanion_8(Game_State *game_state, GLuint category_up, GLuint category_down, v2 companion_size, v2 card_dimension, u32 card_entity_index)
{
    AddCompanion_7(game_state, category_up, category_down, companion_size, card_dimension, card_entity_index);
    
    v2 padding = vec2(0.5f, 0.3f);
    v2 companion_offset = +0.5f * card_dimension - 1.0f * companion_size - padding;
    
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_index, 
                       vec2(0, -0.5f * companion_offset.y));
    
}

internal inline void
AddCompanion_9(Game_State *game_state, GLuint category_up, GLuint category_down, v2 companion_size, v2 card_dimension, u32 card_entity_index)
{
    AddCompanion_Ace(game_state, category_up, companion_size, card_entity_index);
    
    AddCompanion_4(game_state, category_up, category_down, companion_size, card_dimension, card_entity_index);
    
    v2 padding = vec2(0.5f, 0.3f);
    v2 companion_offset = +0.5f * card_dimension - 1.0f * companion_size - padding;
    
    
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_index, 
                       vec2(-companion_offset.x, 0.3f * companion_offset.y));
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_index, 
                       vec2(companion_offset.x, 0.3f * companion_offset.y));
    
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_index, 
                       vec2(-companion_offset.x, -0.3f * companion_offset.y));
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_index, 
                       vec2(companion_offset.x, -0.3f * companion_offset.y));
    
}

internal inline void
AddCompanion_10(Game_State *game_state, GLuint category_up, GLuint category_down, v2 companion_size, v2 card_dimension, u32 card_entity_index)
{
    AddCompanion_4(game_state, category_up, category_down, companion_size, card_dimension, card_entity_index);
    
    v2 padding = vec2(0.5f, 0.3f);
    v2 companion_offset = +0.5f * card_dimension - 1.0f * companion_size - padding;
    
    
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_index, 
                       vec2(-companion_offset.x, 0.3f * companion_offset.y));
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_index, 
                       vec2(companion_offset.x, 0.3f * companion_offset.y));
    
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_index, 
                       vec2(-companion_offset.x, -0.3f * companion_offset.y));
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_index, 
                       vec2(companion_offset.x, -0.3f * companion_offset.y));
    
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_index, 
                       vec2(0, 0.69f * companion_offset.y));
    
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_index, 
                       vec2(0, -0.69f * companion_offset.y));
    
}

internal inline  void
AddCardCompanions(Game_State *game_state, Frensh_Suited_Cards_Texture *frensh_deck, Card_Type card_type, v2 card_dimension, u32 card_entity_index)
{
    GLuint category_up = 0;
    GLuint category_down = 0;
    
    b32 is_black = false;
    switch(card_type.category)
    {
        case Category_Clovers: // black
        {
            is_black = true;
            category_up   = frensh_deck->clovers_up;
            category_down = frensh_deck->clovers_down;
        } break;
        case Category_Hearts:  // red
        {
            is_black = false;
            category_up   = frensh_deck->hearts_up;
            category_down = frensh_deck->hearts_down;
        } break;
        case Category_Tiles:   // red
        {
            is_black = false;
            category_up   = frensh_deck->tiles;
            category_down = frensh_deck->tiles;
        } break;
        case Category_Pikes:   // black
        {
            is_black = true;
            category_up   = frensh_deck->pikes_up;
            category_down = frensh_deck->pikes_down;
        } break;
    }
    
    GLuint number_up; 
    GLuint number_down;
    if (is_black)
    {
        number_up   = frensh_deck->black_numbers_up[card_type.number];
        number_down = frensh_deck->black_numbers_down[card_type.number];
    }
    else
    {
        number_up   = frensh_deck->red_numbers_up[card_type.number];
        number_down = frensh_deck->red_numbers_down[card_type.number];
    }
    
    v2 padding        = 0.6f * vec2(0.5f, 0.5f);
    v2 companion_size = 0.6f * vec2(0.75f, 1.0f);
    v2 companion_offset = +0.5f * card_dimension - 0.5f * companion_size - padding;
    
    AddCompanionEntity(game_state, number_up, companion_size, card_entity_index,
                       vec2(-companion_offset.x, companion_offset.y));
    
    AddCompanionEntity(game_state, number_down, companion_size, card_entity_index,
                       vec2(companion_offset.x, -companion_offset.y));
    
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_index,
                       vec2(-companion_offset.x, companion_offset.y - (padding.y + 0.5f * companion_size.y + 0.1f)));
    
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_index,
                       vec2(companion_offset.x,  -companion_offset.y + (padding.y + 0.5f * companion_size.y + 0.1f)));
    
    
    companion_size = 0.6f * vec2(1.5f, 1.5f);
    switch (card_type.number)
    {
        case Card_Number_Ace:
        {
            AddCompanion_Ace(game_state, category_up, companion_size, card_entity_index);
        } break;
        case Card_Number_2:
        {
            AddCompanion_2(game_state, category_up, category_down, companion_size, card_dimension, card_entity_index);
        } break;
        case Card_Number_3:
        {
            AddCompanion_3(game_state, category_up, category_down, companion_size, card_dimension, card_entity_index);
        } break;
        case Card_Number_4:
        {
            AddCompanion_4(game_state, category_up, category_down, companion_size, card_dimension, card_entity_index);
        } break;
        case Card_Number_5:
        {
            AddCompanion_5(game_state, category_up, category_down, companion_size, card_dimension, card_entity_index);
        } break;
        case Card_Number_6:
        {
            AddCompanion_6(game_state, category_up, category_down, companion_size, card_dimension, card_entity_index);
        } break;
        case Card_Number_7:
        {
            AddCompanion_7(game_state, category_up, category_down, companion_size, card_dimension, card_entity_index);
        } break;
        case Card_Number_8:
        {
            AddCompanion_8(game_state, category_up, category_down, companion_size, card_dimension, card_entity_index);
            
        } break;
        case Card_Number_9:
        {
            AddCompanion_9(game_state, category_up, category_down, companion_size, card_dimension, card_entity_index);
        } break;
        case Card_Number_10:
        {
            AddCompanion_10(game_state, category_up, category_down, companion_size, card_dimension, card_entity_index);
        } break;
        case Card_Number_Jack:
        {
            GLuint jack = frensh_deck->jacks[card_type.category];
            companion_size = card_dimension - companion_size - 2 * padding;
            AddCompanion_Ace(game_state, jack, companion_size, card_entity_index);
        } break;
        case Card_Number_Queen:
        {
            GLuint queen = frensh_deck->queens[card_type.category];
            companion_size = card_dimension - companion_size - 2 * padding;
            AddCompanion_Ace(game_state, queen, companion_size, card_entity_index);
        } break;
        case Card_Number_King:
        {
            GLuint king = frensh_deck->kings[card_type.category];
            companion_size = card_dimension - companion_size - 2 * padding;
            AddCompanion_Ace(game_state, king, companion_size, card_entity_index);
        } break;
    }
    
}

internal void
AddCardEntity(Game_State *game_state, Card_Type card_type, Card_Residency card_residency, b32 is_fliped = false)
{
    u32 card_entity_index = AddEntity(game_state);
    Entity *card = game_state->entities + card_entity_index;
    *card = {};
    card->type = Entity_Type_Entity_Card;
    card->target_dimension   = vec2(CARD_WIDTH, CARD_HEIGHT);
    card->current_dimension  = vec2(CARD_WIDTH, CARD_HEIGHT);
    ChangeResidency(game_state, card_entity_index, card_residency);
    f32 world_width = MAX_UNITS_PER_X;
    f32 world_height = MAX_UNITS_PER_X * game_state->rendering_context.aspect_ratio;
    card->center_pos.xy = 0.5f * vec2(world_width, world_height);
    
    card->dy_angle = 4 * PI;
    
    if (is_fliped)
    {
        card->target_y_angle = PI;
        card->y_angle = PI;
    }
    
#if TEST_ONE_CARD
    card->target_pos.xy = card->center_pos.xy;
    card->residency_pos.xy = card->center_pos.xy;
#endif
    
    card->dDimension = 20.f;
    
    Frensh_Suited_Cards_Texture *frensh_deck = &game_state->rendering_context.frensh_deck;
    card->texture = frensh_deck->card_frame_texture;
    
    AddCardCompanions(game_state, frensh_deck, card_type, card->current_dimension, card_entity_index);
} 



internal Card_Type
MakeCardType(Card_Category category, Card_Number number)
{
    Card_Type result;
    result.category = category;
    result.number   = number;
    return result;
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
        
        AddNullEntity(game_state);
        AddCursorEntity(game_state);
        
#if TEST_ONE_CARD
        AddCardEntity(game_state, MakeCardType(Category_Hearts, Card_Number_10), Card_Residency_Down);
#else
        for (u32 card_index = 0;
             card_index < 13;
             ++card_index)
        {
            AddCardEntity(game_state, MakeCardType(Category_Tiles, (Card_Number)card_index), Card_Residency_Left);
        }
        
        for (u32 card_index = 0;
             card_index < 13;
             ++card_index)
        {
            AddCardEntity(game_state, MakeCardType(Category_Hearts, (Card_Number)card_index), Card_Residency_Down);
        }
        
        for (u32 card_index = 0;
             card_index < 13;
             ++card_index)
        {
            AddCardEntity(game_state, MakeCardType(Category_Clovers, (Card_Number)card_index), Card_Residency_Up);
        }
        
        for (u32 card_index = 0;
             card_index < 13;
             ++card_index)
        {
            AddCardEntity(game_state, MakeCardType(Category_Pikes, (Card_Number)card_index), Card_Residency_Right);
        }
        
        
        for (u32 card_index = 0;
             card_index < 13;
             ++card_index)
        {
            AddCardEntity(game_state, MakeCardType(Category_Pikes, (Card_Number)card_index), Card_Residency_Table, true);
        }
        
        
#endif
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
        
        DebugDrawQuadScreenCoord(rendering_context, vec3(os->mouse_position, 99), vec2(10, 10), vec3(1, .3f, .5f));
        
#if 0
        char buffer[64];
        sprintf(buffer, "frame time : %f", os->dtime);
        s8 frame_time = s8{buffer, CalculateCStringLength(buffer)};
        DebugDrawText(rendering_context, frame_time, vec2(400, 20), vec3(1,1,1));
        sprintf(buffer, "frame fps : %f", 1.0f / os->dtime);
        s8 frame_fps = s8{buffer, CalculateCStringLength(buffer)};
        DebugDrawText(rendering_context, frame_time, vec2(400, 60), vec3(1,1,1));
#endif
        
        if (os->controller.toggle_fullscreen.pressed)
        {
            os->fullscreen ^= 1;
        }
        // NOTE(fakhri): ignore any input i didn't handle this frame
        OS_EmptyEvents();
    }
}