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
HandleAvailableMessages(Game_State *game_state)
{
    Game_Session *game_session = &game_state->game_session;
    for(MessageResult message_result = os->GetNextNetworkMessageIfAvailable();
        message_result.is_available;
        message_result = os->GetNextNetworkMessageIfAvailable()
        )
    {
        switch(message_result.message.type)
        {
            case MessageType_From_Server_PLAYER_JOINED_GAME:
            {
                u32 player_index = message_result.message.player_index;
                game_session->players[player_index] = message_result.message.player_username;
                ++game_session->players_joined_sofar;
            } break;
            case MessageType_From_Server_INVALIDE_USERNAME:
            {
            } break;
            default:
            {
                // NOTE(fakhri): unhandled message
                // NOTE(fakhri): unhandled message
                Assert(false);
            }
        }
    }
}

inline void
FetchHosts(Hosts_Storage *hosts_storage)
{
    os->PushNetworkMessage(CreateFetchAvailableHostsMessage(hosts_storage));
    hosts_storage->is_fetching = true;
}

inline void
OpenMenu(Game_State *game_state, Game_Mode menu_mode)
{
    Assert(Game_Mode_MENU_BEGIN < menu_mode && menu_mode < Game_Mode_MENU_END);
    game_state->game_mode = menu_mode;
    game_state->ui_context.selected_item = -1;
    glDepthFunc(GL_ALWAYS);
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
    entity->velocity += os->game_dt * acceleration;
    entity->center_pos.xy += os->game_dt * entity->velocity + 0.5f * square_f(os->game_dt) * acceleration;
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
    
    entity->y_angle = MoveTowards(entity->y_angle, entity->target_y_angle, entity->dy_angle * os->game_dt);
    
    // NOTE(fakhri): update dimension
    entity->current_dimension = Vec2MoveTowards(entity->current_dimension,
                                                entity->target_dimension,
                                                os->game_dt * entity->dDimension);
    
    MoveEntity(game_state, entity, 100, 10, 0.5f);
}

internal void
UpdateCompanionEntity(Game_State *game_state, Entity *entity)
{
    Assert(entity->followed_entity_index);
    
    MoveEntity(game_state, entity, 85.f, 0.5f, 0.1f);
    
#if 0    
    Entity *followed_entity = game_state->entities + entity->followed_entity_index;
    entity->center_pos.xy = ClampInsideRect(RectCentDim(followed_entity->center_pos.xy, followed_entity->current_dimension), entity->center_pos.xy);
#endif
    
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    ChangeActiveFont(rendering_context, &rendering_context->menu_title_font);
    f32 x = 0.5f * screen.width;
    f32 y = 0.1f * screen.height;
    
    
    // NOTE(fakhri): Menu Title
    UI_Label(game_state, S8Lit("Truth Or Lies?"), x, y);
    
    y = 0.5f * screen.height;
    // NOTE(fakhri): Join Game Button
    ChangeActiveFont(rendering_context, &rendering_context->menu_item_font);
    f32 stride = 1.9f * rendering_context->active_font->font_height;
    
    // NOTE(fakhri): join session button
    if (UI_Button(game_state, S8Lit("Join Game Room"), x, y, vec2(half_screen.width, 1.1f * rendering_context->active_font->font_height)))
    {
        FetchHosts(&game_session->hosts_storage);
        OpenMenu(game_state, Game_Mode_MENU_JOIN_GAME);
    }
    y += stride;
    
    // NOTE(fakhri): Host Session button
    if (UI_Button(game_state, S8Lit("Create Game Room"), x, y, vec2(half_screen.width, 1.1f * rendering_context->active_font->font_height)))
    {
        OpenMenu(game_state, Game_Mode_MENU_WAITING_PLAYERS);
        os->PushNetworkMessage(CreateStartHostServerMessage());
        SetFlag(game_session->session_state_flags, SESSION_FLAG_HOSTING_GAME);
    }
    y += stride;
    
    // NOTE(fakhri): Quit
    if (UI_Button(game_state, S8Lit("Quit"), x, y, vec2(half_screen.width, 1.1f * rendering_context->active_font->font_height)))
    {
        os->quit = 1;
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    ChangeActiveFont(rendering_context, &rendering_context->menu_title_font);
    f32 x = 0.5f * screen.width;
    f32 y = 0.1f * screen.height;
    
    // NOTE(fakhri): Menu Title
    UI_Label(game_state, S8Lit("Choose a host game"), x, y);
    f32 stride = 2.0f * rendering_context->active_font->font_height;
    // TODO(fakhri): draw a list of available hosts here
    b32 is_still_fetching_hosts = game_session->hosts_storage.is_fetching;
    if (!is_still_fetching_hosts)
    {
        y = 0.3f * screen.height;
        for (u32 host_index = 0;
             host_index < game_session->hosts_storage.hosts_count;
             ++host_index)
        {
            Host_Info *host_info = game_session->hosts_storage.hosts + host_index;
            UI_Label(game_state, String8FromCString(host_info->hostname),x, y );
            y += stride;
        }
        
        y = 0.9f * screen.height;
        
        if(UI_Button(game_state, S8Lit("Refresh Hosts List"), x, y, vec2(half_screen.width, 1.1f * rendering_context->active_font->font_height)))
        {
            FetchHosts(&game_session->hosts_storage);
        }
    }
    else
    {
        y = 0.5f * screen.height;
        UI_Label(game_state, S8Lit("Refreshing Available Hosts"), x, y);
    }
    
    UI_EndFrame(ui_context, controller);
}

void UpdateAndRenderUserNameMenu(Game_State *game_state,  Rendering_Context *rendering_context, UI_Context *ui_context, Controller *controller)
{
#if 1
    Game_Session *game_session = &game_state->game_session;
    UI_BeginFrame(ui_context);
    
    v2 half_screen = 0.5f * rendering_context->screen;
    v2 screen = rendering_context->screen;
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    ChangeActiveFont(rendering_context, &rendering_context->menu_title_font);
    f32 x = 0.5f * screen.width;
    f32 y = 0.1f * screen.height;
    
    // NOTE(fakhri): Menu Title
    UI_Label(game_state, S8Lit("Connect to game"), x, y);
    
    ChangeActiveFont(rendering_context, &rendering_context->menu_item_font);
    y = 0.4f * screen.height;
    f32 stride = 2.0f * rendering_context->active_font->font_height;
    
    // NOTE(fakhri): Username input field
    {
        s8 label_text = S8Lit("Your username is?");
        UI_Label(game_state, label_text, x, y);
        y+= stride;
        
        v2 input_field_size = {500, 50};
        UI_InputField(game_state, input_field_size, x, y, &game_state->username_buffer);
        y += stride;
    }
    
    y = 0.7f * screen.height;
    if (!IsFlagSet(game_session->session_state_flags, SESSION_FLAG_TRYING_JOIN_GAME))
    {
        if (UI_Button(game_state, S8Lit("Join"), x, y, vec2(half_screen.width, 1.1f * rendering_context->active_font->font_height)))
        {
            s8 username =  game_state->username_buffer.buffer;
            
            os->PushNetworkMessage(CreateUsernameMessage(username));
            // TODO(fakhri): make sure the username isn't empty
            SetFlag(game_session->session_state_flags, SESSION_FLAG_TRYING_JOIN_GAME);
        }
        
        // TODO(fakhri): just for debug while we don't have a working server
        SetFlag(game_session->session_state_flags, SESSION_FLAG_JOINED_GAME);
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
            // TODO(fakhri): investigate reasons and report them
        }
    }
    UI_EndFrame(ui_context, controller);
#endif
}

internal
void UpdateAndRenderWaitingPlayersMenu(Game_State *game_state,  Rendering_Context *rendering_context, UI_Context *ui_context, Controller *controller)
{
#if 1
    Game_Session *game_session = &game_state->game_session;
    v2 half_screen = 0.5f * rendering_context->screen;
    v2 screen = rendering_context->screen;
    
    UI_BeginFrame(ui_context);
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    v3 white = vec3(1.0f, 1.0f, 1.0f);
    v3 none_white = 0.3f * white;
    
    
    ChangeActiveFont(rendering_context, &rendering_context->menu_title_font);
    f32 x = 0.5f * screen.width;
    f32 y = 0.1f * screen.height;
    
    // NOTE(fakhri): render menu title
    UI_Label(game_state, S8Lit("Joined Players"), x, y);
    
    // NOTE(fakhri): use menu item fonts
    ChangeActiveFont(rendering_context, &rendering_context->menu_item_font);
    y = 0.2f * screen.height;
    f32 stride = 2.0f * rendering_context->active_font->font_height;
    
    if (IsFlagSet(game_session->session_state_flags, SESSION_FLAG_HOSTING_GAME))
    {
        UI_Label(game_state, S8Lit("You are hosting the game"), x, y);
        y += stride;
        
        // TODO(fakhri): format this text
        s8 server_addres_message = S8Lit("your address is 127.0.0.1");
        x = 0.3f * screen.width; 
        UI_Label(game_state, server_addres_message, x, y);
        
        x = 0.7f * screen.width;  
        if (UI_Button(game_state, S8Lit("Copy to Clipboard"), x, y, vec2(half_screen.width, 1.1f * rendering_context->active_font->font_height)))
        {
            // TODO(fakhri): copy server address to clipboard
        }
        x = 0.5f * screen.width;
        y += stride;
    }
    
    UI_Label(game_state, S8Lit("Connected players"), x, y);
    y += stride;
    
    for (u32 players_index = 0;
         players_index < PLAYERS_COUNT;
         ++players_index)
    {
        UI_Label(game_state, S8Lit("blabla"), x , y);
        //UI_Label(game_state, game_session->players[players_index], x , y);
        y += stride;
    }
    
    y = 0.8f * screen.height;
    UI_Label(game_state, S8Lit("Waiting Players to Join"), x, y, true);
    y += stride;
    
    if (UI_Button(game_state, S8Lit("Cancel"), x, y, vec2(half_screen.width, 1.1f * rendering_context->active_font->font_height)))
    {
        if (IsFlagSet(game_session->session_state_flags, SESSION_FLAG_HOSTING_GAME))
        {
            // TODO(fakhri): display a warning message
        }
        else
        {
            // TODO(fakhri): dispaly a confirmation message
        }
        
        b32 cancled_join_game_confirmed = true;
        if (cancled_join_game_confirmed)
        {
            OpenMenu(game_state, Game_Mode_MENU_MAIN);
            
            if (IsFlagSet(game_session->session_state_flags, SESSION_FLAG_HOSTING_GAME))
            {
                // TODO(fakhri): stop the server
            }
        }
    }
    
    if (game_session->players_joined_sofar == PLAYERS_COUNT)
    {
        // NOTE(fakhri): enough players here
        game_state->game_mode = Game_Mode_GAME;
        glDepthFunc(GL_LESS);
    }
    
#endif
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
    
    // TODO(fakhri): experiment with each companion entity has it's own differnet speed
    
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
        
        game_state->host_address_buffer = InitBuffer(&os->permanent_arena, SERVER_ADDRESS_BUFFER_SIZE);
        game_state->username_buffer = InitBuffer(&os->permanent_arena, USERNAME_BUFFER_SIZE);
        
        // TODO(fakhri): this is just for debug
        OpenMenu(game_state, Game_Mode_MENU_MAIN);
        game_state->game_mode = Game_Mode_GAME;
        game_state->game_session = {};
        //game_state->game_session.players_joined_sofar = PLAYERS_COUNT;
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
    
    APP_UPDATE
    {
        Game_State *game_state = os->game_state;
        Rendering_Context *rendering_context = &game_state->rendering_context;
        UI_Context *ui_context = &game_state->ui_context;
        Controller *controller = &os->controller;
        
        HandleAvailableMessages(game_state);
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
        
#if 1
        ChangeActiveFont(rendering_context, &rendering_context->arial_font);
        char buffer[64];
        sprintf(buffer, "frame time : %f", os->real_dt);
        s8 frame_time = s8{buffer, CalculateCStringLength(buffer)};
        DebugDrawTextScreenCoord(rendering_context, frame_time, vec2(400, 20), vec3(1,1,1));
        sprintf(buffer, "frame fps : %f", 1.0f / os->real_dt);
        s8 frame_fps = s8{buffer, CalculateCStringLength(buffer)};
        DebugDrawTextScreenCoord(rendering_context, frame_time, vec2(400, 60), vec3(1,1,1));
#endif
        
        if (os->controller.toggle_fullscreen.pressed)
        {
            os->fullscreen ^= 1;
        }
        // NOTE(fakhri): ignore any input i didn't handle this frame
        OS_EmptyEvents();
    }
}