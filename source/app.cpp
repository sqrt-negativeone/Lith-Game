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
#include "entity.cpp"


internal void
AssignResidencyToPlayers(Game_State *game_state, Game_Session *game_session)
{
    u32 residency = Card_Residency_Left;
    for(u32 player_index = 0;
        player_index < ArrayCount(game_session->players);
        ++player_index)
    {
        Player *player = game_session->players + player_index;
        if (player_index == game_session->my_player_id)
        {
            player->assigned_residency = Card_Residency_Down;
            game_state->residencies[Card_Residency_Down].controlling_player_id = player_index;
        }
        else
        {
            Assert(residency != Card_Residency_Down);
            player->assigned_residency = (Card_Residency)residency;
            game_state->residencies[residency].controlling_player_id = player_index;
            ++residency;
        }
    }
}

internal inline void
AddPlayer(Game_Session *game_session, MessagePlayer *message_player, u32 player_id)
{
    Player *player = game_session->players + player_id;
    player->username = PushStringF(&os->permanent_arena, "%s", message_player->username);
    player->joined = true;
    ++game_session->players_joined_so_far;
}

internal void
HandleAvailableMessages(Game_State *game_state, Game_Session *game_session)
{
    for(MessageResult message_result = os->GetNextNetworkMessageIfAvailable();
        message_result.is_available;
        message_result = os->GetNextNetworkMessageIfAvailable()
        )
    {
        switch(message_result.message.type)
        {
            case MessageType_From_Host_New_Player_Joined:
            {
                u32 player_id = message_result.message.player_id;
                MessagePlayer *message_player = message_result.message.players + player_id;
                AddPlayer(game_session, message_player, player_id);
            } break;
            case MessageType_From_Host_Connected_Players_List:
            {
                // NOTE(fakhri): we receive this message when we first join the host
                // the host sends a list of all the currently connected players,
                // and we are garanteed to be the last one on this list,
                // so we can safely assume that our id is `player_count - 1`
                game_session->my_player_id = message_result.message.players_count - 1;
                for(u32 player_index = 0;
                    player_index < message_result.message.players_count;
                    ++player_index)
                {
                    MessagePlayer *message_player = message_result.message.players + player_index;
                    AddPlayer(game_session, message_player, player_index);
                }
                SetFlag(game_session->flags, SESSION_FLAG_JOINED_GAME);
            } break;
            case MessageType_From_Host_Invalid_Username:
            {
                SetFlag(game_session->flags, SESSION_FLAG_FAILED_JOIN_GAME);
            } break;
            case MessageType_From_Host_Shuffled_Deck:
            {
                AssignResidencyToPlayers(game_state, game_session);
                // NOTE(fakhri): split the deck between players
                for(u32 player_index = 0;
                    player_index < MAX_PLAYER_COUNT;
                    ++player_index)
                {
                    Player *player = game_session->players + player_index;
                    u32 card_base = player_index * CARDS_PER_PLAYER;
                    for(u32 card_offset = 0;
                        card_offset < CARDS_PER_PLAYER;
                        ++card_offset)
                    {
                        u32 compact_card_index = card_base + card_offset;
                        Card_Type card_type = UnpackCompactCardType(message_result.message.compact_deck[compact_card_index]);
                        b32 is_flipped = (player_index != game_session->my_player_id);
                        AddCardEntity(game_state, card_type, player->assigned_residency, is_flipped);
                    }
                    
                }
                SetFlag(game_session->flags, SESSION_FLAG_HOST_FINISHED_SPLITTING_DECK);
            } break;
            case MessageType_From_Host_Change_Player_Turn:
            {
                // TODO(fakhri): change the current player turn
            } break;
            default:
            {
                // NOTE(fakhri): unhandled message
                // NOTE(fakhri): unhandled message
                Assert(!"MESSAGE NOT IMPLEMENTED YET");
            }
        }
    }
}

internal inline void
FetchHosts(Hosts_Storage *hosts_storage)
{
    os->PushNetworkMessage(CreateFetchAvailableHostsMessage(hosts_storage));
    hosts_storage->is_fetching = true;
}

internal inline void
OpenMenu(Game_State *game_state, Game_Mode menu_mode)
{
    Assert(Game_Mode_MENU_BEGIN < menu_mode && menu_mode < Game_Mode_MENU_END);
    game_state->game_mode = menu_mode;
    game_state->ui_context.selected_item = -1;
    glDepthFunc(GL_ALWAYS);
}

internal void
StartGame(Game_State *game_state, Game_Session *game_session)
{
    game_state->game_mode = Game_Mode_GAME;
    SetFlag(game_session->flags, SESSION_FLAG_HOST_SPLITTING_DECK);
    glDepthFunc(GL_LESS);
}

internal void
UpdateAndRenderGame(Game_State *game_state, Rendering_Context *rendering_context, Controller *controller)
{
    // TODO(fakhri): we need to have a concept of stuff taking sometime to happen
    // and not just instantaneous
    // TODO(fakhri): how should we display the message?
    Game_Session *game_session = &game_state->game_session;
    glClearColor(0.1f, 0.1f, 0.1f, 1.f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    v3 white = vec3(1, 1, 1);
    v3 red = vec3(1, 0, 0);
    
    // TODO(fakhri): render a background
    if(IsFlagSet(game_session->flags, SESSION_FLAG_HOST_SPLITTING_DECK))
    {
        // NOTE(fakhri): we wait
        ChangeActiveFont(rendering_context, &rendering_context->arial_font);
        DebugDrawTextScreenCoord(rendering_context, S8Lit("host splitting the deck"), 0.5f * rendering_context->screen, vec3(1,1,1));
        
        if(IsFlagSet(game_session->flags, SESSION_FLAG_HOST_FINISHED_SPLITTING_DECK))
        {
            ClearFlag(game_session->flags, SESSION_FLAG_HOST_SPLITTING_DECK);
        }
    }
    else
    {
        // TODO(fakhri): block user control when there are cards that should get burnt
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
                } break;
                case Entity_Type_Card:
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
                case Entity_Type_Companion:
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
        
        if (game_state->should_burn_cards)
        {
            game_state->time_to_burn_cards -= os->game_dt;
            if(game_state->time_to_burn_cards <= 0)
            {
                game_state->should_burn_cards = false;
                
                // NOTE(fakhri): burn the marked cards
                Assert(Card_Residency_Down - Card_Residency_Left + 1  == MAX_PLAYER_COUNT);
                for(u32 residency_index = Card_Residency_Left;
                    residency_index <= Card_Residency_Down;
                    ++residency_index)
                {
                    Residency *residency = game_state->residencies + residency_index;
                    for(u32 index_in_residency = 0;
                        index_in_residency < residency->entity_count;
                        ++index_in_residency)
                    {
                        u32 entity_index = residency->entity_indices[index_in_residency];
                        Entity *entity = game_state->entities + entity_index;
                        if(entity->marked_for_burning)
                        {
                            ChangeResidency(game_state, entity_index, Card_Residency_Burnt);
                        }
                    }
                }
            }
        }
        
        // TODO(fakhri): think about how to render the players usernames
        for(u32 player_index = 0;
            player_index < MAX_PLAYER_COUNT;
            ++player_index)
        {
            Player *player = game_session->players + player_index;
            v2 render_position;
            switch(player->assigned_residency)
            {
                case Card_Residency_Left:
                {
                    render_position = vec2(0,0);
                } break;
                case Card_Residency_Right:
                {
                    render_position = vec2(0,0);
                } break;
                case Card_Residency_Up:
                {
                    render_position = vec2(0, 0);
                } break;
                case Card_Residency_Down:
                {
                    render_position = vec2(0, 0);
                } break;
                default:
                {
                    Assert(!"PLAYER SHOULD NOT BE ASSIGNED THIS RESIDENCY");
                    render_position = vec2(0, 0);
                }
            }
            DebugDrawTextWorldCoord(rendering_context, player->username, render_position, white);
        }
    }
    
    // @DebugOnly
    {
        if (controller->right_mouse.pressed)
        {
            for (u32 residency_type = Card_Residency_Left;
                 residency_type <= Card_Residency_Burnt;
                 ++residency_type)
            {
                ReorganizeResidencyCards(game_state, (Card_Residency)residency_type);
            }
        }
        
        if (controller->confirm.pressed)
        {
            AddDebugEntites(game_state);
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
#if 0
        FetchHosts(&game_session->hosts_storage);
        OpenMenu(game_state, Game_Mode_MENU_JOIN_GAME);
#else
        os->PushNetworkMessage(CreateConnectToServerMessage(S8Lit("")));
        OpenMenu(game_state, Game_Mode_MENU_USERNAME);
#endif
    }
    y += stride;
    
#if 0    
    // NOTE(fakhri): Host Session button
    if (UI_Button(game_state, S8Lit("Create Game Room"), x, y, vec2(half_screen.width, 1.1f * rendering_context->active_font->font_height)))
    {
        OpenMenu(game_state, Game_Mode_MENU_WAITING_PLAYERS);
        os->PushNetworkMessage(CreateStartHostServerMessage());
        SetFlag(game_session->session_state_flags, SESSION_FLAG_HOSTING_GAME);
    }
    y += stride;
#endif
    
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
        y += stride;
        
        v2 input_field_size = {500, 50};
        UI_InputField(game_state, input_field_size, x, y, &game_state->username_buffer);
        y += stride;
    }
    
    y = 0.7f * screen.height;
    if (!IsFlagSet(game_session->flags, SESSION_FLAG_TRYING_JOIN_GAME))
    {
        if (UI_Button(game_state, S8Lit("Join"), x, y, vec2(half_screen.width, 1.1f * rendering_context->active_font->font_height)))
        {
            s8 username =  game_state->username_buffer.buffer;
            
            if(username.size)
            {
                os->PushNetworkMessage(CreateUsernameMessage(username));
                SetFlag(game_session->flags, SESSION_FLAG_TRYING_JOIN_GAME);
            }
            else
            {
                // TODO(fakhri): dispaly error message
            }
        }
    }
    else
    {
        s8 label_text = S8Lit("joinning the game");
        UI_Label(game_state, label_text, x, y, true);
        
        if (IsFlagSet(game_session->flags, SESSION_FLAG_JOINED_GAME))
        {
            // NOTE(fakhri): good, we wait for all players to join and then we start the game
            OpenMenu(game_state, Game_Mode_MENU_WAITING_PLAYERS);
            ClearFlag(game_session->flags, SESSION_FLAG_TRYING_JOIN_GAME);
        }
        
        if (IsFlagSet(game_session->flags, SESSION_FLAG_FAILED_JOIN_GAME))
        {
            // TODO(fakhri): display an error message
            ClearFlag(game_session->flags, SESSION_FLAG_TRYING_JOIN_GAME);
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
    
    UI_Label(game_state, S8Lit("Connected players"), x, y);
    y += stride;
    
    // NOTE(fakhri): render the connected players
    for (u32 player_index = 0;
         player_index < ArrayCount(game_session->players);
         ++player_index)
    {
        Player *player = game_session->players + player_index;
        if(player->joined)
        {
            UI_Label(game_state, player->username, x , y);
        }
        else
        {
            UI_Label(game_state, S8Lit("waiting player"), x , y, true);
        }
        y += stride;
    }
    
    y = 0.8f * screen.height;
    UI_Label(game_state, S8Lit("Waiting Players to Join"), x, y, true);
    y += stride;
    
    if (UI_Button(game_state, S8Lit("Cancel"), x, y, vec2(half_screen.width, 1.1f * rendering_context->active_font->font_height)))
    {
        if (IsFlagSet(game_session->flags, SESSION_FLAG_HOSTING_GAME))
        {
            // TODO(fakhri): display a warning message
        }
        else
        {
            // TODO(fakhri): dispaly a confirmation message
        }
        
        // TODO(fakhri): implement confirmation message
        b32 cancled_join_game_confirmed = true;
        if (cancled_join_game_confirmed)
        {
            OpenMenu(game_state, Game_Mode_MENU_MAIN);
            
            if (IsFlagSet(game_session->flags, SESSION_FLAG_HOSTING_GAME))
            {
                // TODO(fakhri): stop the server
            }
            
            game_session->players_joined_so_far = 0;
        }
    }
    
    if (game_session->players_joined_so_far == MAX_PLAYER_COUNT)
    {
        // NOTE(fakhri): enough players have joined
        StartGame(game_state, game_session);
    }
    
#endif
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
        
        OpenMenu(game_state, Game_Mode_MENU_MAIN);
        
        InitResidencies(game_state);
        
        // @DebugOnly
#if 1
        glDepthFunc(GL_LESS);
        for(u32 residency_index = Card_Residency_Left;
            residency_index <= Card_Residency_Down;
            ++residency_index)
        {
            game_state->residencies[residency_index].controlling_player_id = 0;
        }
        AddDebugEntites(game_state);
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
        Game_Session *game_session = &game_state->game_session;
        Rendering_Context *rendering_context = &game_state->rendering_context;
        UI_Context *ui_context = &game_state->ui_context;
        game_state->controller = {};
        Controller *controller = &game_state->controller;
        
        HandleAvailableMessages(game_state, game_session);
        UpdateScreenSize(rendering_context);
        
        // NOTE(fakhri): handle input
        {
            OS_Event *event = 0;
            while(OS_GetNextEvent(&event))
            {
                if (OS_EventIsMouse(event))
                {
                    switch(event->type)
                    {
                        case OS_EventType_MousePress:
                        {
                            switch(event->mouse_button)
                            {
                                case MouseButton_Left: controller->left_mouse.pressed  = true; break;
                                case MouseButton_Right:controller->right_mouse.pressed = true; break;
                                default: break;
                            }
                        } break;
                        case OS_EventType_MouseRelease:
                        {
                            switch(event->mouse_button)
                            {
                                case MouseButton_Left: controller->left_mouse.released  = true; break;
                                case MouseButton_Right:controller->right_mouse.released = true; break;
                                default: break;
                            }
                        } break;
                        default: break;
                    }
                }
                else
                {
                    switch(event->type)
                    {
                        case OS_EventType_KeyPress:
                        {
                            switch(event->key)
                            {
                                case Key_Esc:   controller->escape_key.pressed        = true; break;
                                case Key_Enter: controller->confirm.pressed           = true; break;
                                case Key_Up:    controller->move_up.pressed           = true; break;
                                case Key_Down:  controller->move_down.pressed         = true; break;
                                case Key_Left:  controller->move_left.pressed         = true; break;
                                case Key_Right: controller->move_right.pressed        = true; break;
                                case Key_F1:    controller->toggle_fullscreen.pressed = true; break;
                                default: break;
                            }
                        } break;
                        case OS_EventType_KeyRelease:
                        {
                            switch(event->key)
                            {
                                case Key_Esc:   controller->escape_key.released        = true; break;
                                case Key_Enter: controller->confirm.released           = true; break;
                                case Key_Up:    controller->move_up.released           = true; break;
                                case Key_Down:  controller->move_down.released         = true; break;
                                case Key_Left:  controller->move_left.released         = true; break;
                                case Key_Right: controller->move_right.released        = true; break;
                                case Key_Space: controller->toggle_fullscreen.released = true; break;
                                default: break;
                            }
                        } break;
                        default: break;
                    }
                }
            }
        }
        
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
        ChangeActiveFont(rendering_context, &rendering_context->arial_font);
        char buffer[64];
        sprintf(buffer, "frame time : %f", os->real_dt);
        s8 frame_time = s8{buffer, CalculateCStringLength(buffer)};
        DebugDrawTextScreenCoord(rendering_context, frame_time, vec2(400, 20), vec3(1,1,1));
        sprintf(buffer, "frame fps : %f", 1.0f / os->real_dt);
        s8 frame_fps = s8{buffer, CalculateCStringLength(buffer)};
        DebugDrawTextScreenCoord(rendering_context, frame_time, vec2(400, 60), vec3(1,1,1));
#endif
        
        if (game_state->controller.toggle_fullscreen.pressed)
        {
            os->fullscreen ^= 1;
        }
        // NOTE(fakhri): ignore any input i didn't handle this frame
        OS_EmptyEvents();
    }
}