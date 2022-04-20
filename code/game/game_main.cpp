
////////////////////////////////
//~ NOTE(fakhri): headers

#include "base/base_inc.h"
#include "network/network_inc.h"
#include "game/game_inc.h"
#include "os/os_inc.h"

////////////////////////////////
//~ NOTE(fakhri): implementations
#include "base/base_inc.cpp"
#include "os/os_inc.cpp"
#include "game_inc.cpp"
#include "network/network_inc.cpp"

global Thread_Ctx game_tctx;

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
    player->username = PushStr8F(os->permanent_arena, "%s", message_player->username);
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
UpdateAndRenderGame(Game_State *game_state, Controller *controller, f32 dt)
{
    
    // TODO(fakhri): we need to have a concept of stuff taking sometime to happen
    // and not just instantaneous
    // TODO(fakhri): how should we display the message?
    Game_Session *game_session = &game_state->game_session;
    v4 white = Vec4(1, 1, 1,1);
    v4 red = Vec4(1, 0, 0, 1);
    
    // TODO(fakhri): render a background
    if(HasFlag(game_session->flags, SESSION_FLAG_HOST_SPLITTING_DECK))
    {
        // NOTE(fakhri): we wait
        ChangeActiveFont(&game_state->render_context, FontKind_Arial);
        
        Render_PushText(&game_state->render_context, Str8Lit("host splitting the deck"), Vec3(0, 0, 60), Vec4(1,0,1,1), CoordinateType_World, FontKind_Arial);
        
        if(HasFlag(game_session->flags, SESSION_FLAG_HOST_FINISHED_SPLITTING_DECK))
        {
            RemoveFlag(game_session->flags, SESSION_FLAG_HOST_SPLITTING_DECK);
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
                    UpdateCardEntity(game_state, entity_index, dt);
                    if (entity->is_pressed)
                    {
                        Render_PushQuad(&game_state->render_context, 
                                        Vec3(entity->center_pos.xy, entity->center_pos.z - 0.01f),
                                        1.05f * entity->current_dimension, red, CoordinateType_World, entity->y_angle);
                        
                    }
                    
                    Render_PushImage(&game_state->render_context, entity->texture, entity->center_pos, entity->current_dimension, CoordinateType_World,
                                     entity->y_angle);
                    
                    v3 card_back_pos = entity->center_pos;
                    
                    Render_PushImage(&game_state->render_context, game_state->frensh_deck.card_back_texture, entity->center_pos, entity->current_dimension, CoordinateType_World, PI32 - entity->y_angle);
                    
                } break;
                case Entity_Type_Companion:
                {
                    UpdateCompanionEntity(game_state, entity, dt);
                    Entity *entity_to_follow = game_state->entities + entity->entity_index_to_follow;
                    Assert(entity_to_follow);
                    
                    Render_PushImage(&game_state->render_context, entity->texture, Vec3(entity->center_pos.xy, entity_to_follow->center_pos.z + 0.01f), entity->current_dimension, CoordinateType_World,
                                     entity_to_follow->y_angle);
                    
                } break;
                default:
                {
                    StopExecution;
                } break;
            }
        }
        
        if (game_state->should_burn_cards)
        {
            game_state->time_to_burn_cards -= dt;
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


#if 0
internal
void UpdateAndRenderMainMenu(Game_State *game_state, UI_Context *ui_context, Controller *controller)
{
    Game_Session *game_session = &game_state->game_session;
    v2 half_screen = 0.5f * game_state->screen;
    v2 screen = game_state->screen;
    
    UI_BeginFrame(ui_context);
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    ChangeActiveFont(game_state, FontKind_MenuTitle);
    f32 x = 0.5f * screen.width;
    f32 y = 0.1f * screen.height;
    
    
    // NOTE(fakhri): Menu Title
    UI_Label(game_state, Str8Lit("Truth Or Lies?"), x, y);
    
    y = 0.5f * screen.height;
    // NOTE(fakhri): Join Game Button
    ChangeActiveFont(game_state, FontKind_MenuItem);
    f32 stride = 1.9f * GetFontHeight(game_state->active_font);
    
    // NOTE(fakhri): join session button
    if (UI_Button(game_state, Str8Lit("Join Game Room"), x, y, Vec2(half_screen.width, 1.1f * GetFontHeight(game_state->active_font))))
    {
#if 0
        FetchHosts(&game_session->hosts_storage);
        OpenMenu(game_state, Game_Mode_MENU_JOIN_GAME);
#else
        os->PushNetworkMessage(CreateConnectToServerMessage(Str8Lit("")));
        OpenMenu(game_state, Game_Mode_MENU_USERNAME);
#endif
    }
    y += stride;
    
#if 0    
    // NOTE(fakhri): Host Session button
    if (UI_Button(game_state, Str8Lit("Create Game Room"), x, y, Vec2(half_screen.width, 1.1f * GetFontHeight(game_state->active_font))))
    {
        OpenMenu(game_state, Game_Mode_MENU_WAITING_PLAYERS);
        os->PushNetworkMessage(CreateStartHostServerMessage());
        SetFlag(game_session->session_state_flags, SESSION_FLAG_HOSTING_GAME);
    }
    y += stride;
#endif
    
    // NOTE(fakhri): Quit
    if (UI_Button(game_state, Str8Lit("Quit"), x, y, Vec2(half_screen.width, 1.1f * GetFontHeight(game_state->active_font))))
    {
        os->quit = 1;
    }
    
    UI_EndFrame(ui_context, controller);
}

void UpdateAndRenderJoinSessionMenu(Game_State *game_state, UI_Context *ui_context, Controller *controller)
{
    Game_Session *game_session = &game_state->game_session;
    
    UI_BeginFrame(ui_context);
    v2 half_screen = 0.5f * game_state->screen;
    v2 screen = game_state->screen;
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    ChangeActiveFont(game_state, FontKind_MenuTitle);
    f32 x = 0.5f * screen.width;
    f32 y = 0.1f * screen.height;
    
    // NOTE(fakhri): Menu Title
    UI_Label(game_state, Str8Lit("Choose a host game"), x, y);
    f32 stride = 2.0f * GetFontHeight(game_state->active_font);
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
            UI_Label(game_state, Str8C(host_info->hostname),x, y );
            y += stride;
        }
        
        y = 0.9f * screen.height;
        
        if(UI_Button(game_state, Str8Lit("Refresh Hosts List"), x, y, Vec2(half_screen.width, 1.1f * GetFontHeight(game_state->active_font))))
        {
            FetchHosts(&game_session->hosts_storage);
        }
    }
    else
    {
        y = 0.5f * screen.height;
        UI_Label(game_state, Str8Lit("Refreshing Available Hosts"), x, y);
    }
    
    UI_EndFrame(ui_context, controller);
}

void UpdateAndRenderUserNameMenu(Game_State *game_state, UI_Context *ui_context, Controller *controller)
{
#if 1
    Game_Session *game_session = &game_state->game_session;
    UI_BeginFrame(ui_context);
    
    v2 half_screen = 0.5f * game_state->screen;
    v2 screen = game_state->screen;
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    ChangeActiveFont(game_state, FontKind_MenuTitle);
    f32 x = 0.5f * screen.width;
    f32 y = 0.1f * screen.height;
    
    // NOTE(fakhri): Menu Title
    UI_Label(game_state, Str8Lit("Connect to game"), x, y);
    
    ChangeActiveFont(game_state, FontKind_MenuItem);
    y = 0.4f * screen.height;
    f32 stride = 2.0f * GetFontHeight(game_state->active_font);
    
    // NOTE(fakhri): Username input field
    {
        String8 label_text = Str8Lit("Your username is?");
        UI_Label(game_state, label_text, x, y);
        y += stride;
        
        v2 input_field_size = {500, 50};
        UI_InputField(game_state, input_field_size, x, y, &game_state->username_buffer);
        y += stride;
    }
    
    y = 0.7f * screen.height;
    if (!HasFlag(game_session->flags, SESSION_FLAG_TRYING_JOIN_GAME))
    {
        if (UI_Button(game_state, Str8Lit("Join"), x, y, Vec2(half_screen.width, 1.1f * GetFontHeight(game_state->active_font))))
        {
            String8 username =  game_state->username_buffer.content;
            
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
        String8 label_text = Str8Lit("joinning the game");
        UI_Label(game_state, label_text, x, y, true);
        
        if (HasFlag(game_session->flags, SESSION_FLAG_JOINED_GAME))
        {
            // NOTE(fakhri): good, we wait for all players to join and then we start the game
            OpenMenu(game_state, Game_Mode_MENU_WAITING_PLAYERS);
            RemoveFlag(game_session->flags, SESSION_FLAG_TRYING_JOIN_GAME);
        }
        
        if (HasFlag(game_session->flags, SESSION_FLAG_FAILED_JOIN_GAME))
        {
            // TODO(fakhri): display an error message
            RemoveFlag(game_session->flags, SESSION_FLAG_TRYING_JOIN_GAME);
        }
    }
    UI_EndFrame(ui_context, controller);
#endif
}

internal
void UpdateAndRenderWaitingPlayersMenu(Game_State *game_state, UI_Context *ui_context, Controller *controller)
{
#if 1
    Game_Session *game_session = &game_state->game_session;
    v2 half_screen = 0.5f * game_state->screen;
    v2 screen = game_state->screen;
    
    UI_BeginFrame(ui_context);
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    v3 white = Vec3(1.0f, 1.0f, 1.0f);
    v3 none_white = 0.3f * white;
    
    
    ChangeActiveFont(game_state, FontKind_MenuTitle);
    f32 x = 0.5f * screen.width;
    f32 y = 0.1f * screen.height;
    
    // NOTE(fakhri): render menu title
    UI_Label(game_state, Str8Lit("Joined Players"), x, y);
    
    // NOTE(fakhri): use menu item fonts
    ChangeActiveFont(game_state, FontKind_MenuItem);
    y = 0.2f * screen.height;
    f32 stride = 2.0f * GetFontHeight(game_state->active_font);
    
    UI_Label(game_state, Str8Lit("Connected players"), x, y);
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
            UI_Label(game_state, Str8Lit("waiting player"), x , y, true);
        }
        y += stride;
    }
    
    y = 0.8f * screen.height;
    UI_Label(game_state, Str8Lit("Waiting Players to Join"), x, y, true);
    y += stride;
    
    if (UI_Button(game_state, Str8Lit("Cancel"), x, y, Vec2(half_screen.width, 1.1f * GetFontHeight(game_state->active_font))))
    {
        if (HasFlag(game_session->flags, SESSION_FLAG_HOSTING_GAME))
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
            
            if (HasFlag(game_session->flags, SESSION_FLAG_HOSTING_GAME))
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
#endif


exported
APP_HotLoad(HotLoad)
{
    os = _os;
    // NOTE(fakhri): init the game thread context
    {
        game_tctx = MakeTCTX();
        SetTCTX(&game_tctx);
    }
    
    gladLoadGL();
}

exported
APP_PermanantLoad(PermanentLoad)
{
    HotLoad(_os);
    
    glEnable(GL_CULL_FACE); 
    
    //glEnable(GL_DEPTH_TEST);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    InitRenderer(&game_state->render_context);
    LoadFrenshSuitedDeck(&game_state->frensh_deck);
    
    UI_Init(&game_state->ui_context, &game_state->render_context);
    
    game_state->host_address_buffer = InitBuffer(os->permanent_arena, SERVER_ADDRESS_BUFFER_SIZE);
    game_state->username_buffer = InitBuffer(os->permanent_arena, USERNAME_BUFFER_SIZE);
    
    OpenMenu(game_state, Game_Mode_MENU_MAIN);
    
    InitResidencies(game_state);
    game_state->time_scale_factor = 1.f;
    game_state->message_to_display = InitBuffer(os->permanent_arena, Kilobytes(1));
    
    // @DebugOnly
#if 1
    //glDepthFunc(GL_LESS);
    for(u32 residency_index = Card_Residency_Left;
        residency_index <= Card_Residency_Down;
        ++residency_index)
    {
        game_state->residencies[residency_index].controlling_player_id = 0;
    }
    AddDebugEntites(game_state);
#endif
}

exported
APP_HotLoadShader(HotLoadShader)
{
    // NOTE(fakhri): compute hash
    u32 hash_index  = ComputeHashShaderPath(shader_name);
    hash_index %= ArrayCount(game_state->render_context.shaders_hash.shader_slots);
    
    b32 found = false;
    for (Shader_Hash_Slot *shader_hash_slot = game_state->render_context.shaders_hash.shader_slots[hash_index];
         shader_hash_slot;
         shader_hash_slot = shader_hash_slot->next_in_hash)
    {
        if (Str8Match(shader_name, shader_hash_slot->shader_name, MatchFlag_CaseInsensitive))
        {
            found = true;
            // NOTE(fakhri): recompile the shader
            Compile_Shader_Result compile_result = CompileShader(shader_name);
            Assert(shader_hash_slot->kind < ArrayCount(game_state->render_context.shaders));
            Shader_Kind shader_kind = shader_hash_slot->kind;
            if (compile_result.is_valid)
            {
                glDeleteProgram(game_state->render_context.shaders[shader_kind].id);
                game_state->render_context.shaders[shader_kind].id = compile_result.program_id;
                SetupShader(&game_state->render_context, shader_kind);
            }
            else
            {
                LogError("Couldn't Recompile Shader %s", shader_name.cstr);
            }
        }
    }
    
    if (!found)
    {
        Log("Make sure that you added %s to the hashtable, couldn't find it", shader_name.cstr);
    }
}


exported
APP_HotUnload(HotUnload)
{
    DeleteTCTX(&game_tctx);
    SetTCTX(0);
}


exported
APP_UpdateAndRender(UpdateAndRender)
{
    glClearColor(0.4f, 0.4f, 0.4f, 1.f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    Game_Session *game_session = &game_state->game_session;
    UI_Context *ui_context = &game_state->ui_context;
    game_state->controller = {};
    Controller *controller = &game_state->controller;
    
    HandleAvailableMessages(game_state, game_session);
    Render_Begin(&game_state->render_context, OS_FrameArena());
    
    // NOTE(fakhri): handle input
    {
        OS_Event *event = os->events.first;
        while(event)
        {
            switch(event->kind)
            {
                case OS_EventKind_Press:
                {
                    switch(event->key)
                    {
                        case OS_Key_Esc:       controller->escape_key.pressed        = true; break;
                        case OS_Key_Enter:     controller->confirm.pressed           = true; break;
                        case OS_Key_Up:        controller->move_up.pressed           = true; break;
                        case OS_Key_Down:      controller->move_down.pressed         = true; break;
                        case OS_Key_Left:      controller->move_left.pressed         = true; break;
                        case OS_Key_Right:     controller->move_right.pressed        = true; break;
                        case OS_Key_F1:        controller->toggle_fullscreen.pressed = true; break;
                        case OS_Key_MouseLeft: controller->left_mouse.pressed        = true; break;
                        case OS_Key_MouseRight:controller->right_mouse.pressed       = true; break;
                        default: break;
                    }
                } break;
                case OS_EventKind_Release:
                {
                    switch(event->key)
                    {
                        case OS_Key_Esc:       controller->escape_key.released        = true; break;
                        case OS_Key_Enter:     controller->confirm.released           = true; break;
                        case OS_Key_Up:        controller->move_up.released           = true; break;
                        case OS_Key_Down:      controller->move_down.released         = true; break;
                        case OS_Key_Left:      controller->move_left.released         = true; break;
                        case OS_Key_Right:     controller->move_right.released        = true; break;
                        case OS_Key_Space:     controller->toggle_fullscreen.released = true; break;
                        case OS_Key_MouseLeft: controller->left_mouse.released        = true; break;
                        case OS_Key_MouseRight:controller->right_mouse.released       = true; break;
                        default: break;
                    }
                } break;
                case OS_EventKind_MouseScroll:
                {
                    // NOTE(fakhri): scale time
                    game_state->time_scale_factor += 0.0001f * event->scroll.y;
                    if (game_state->time_scale_factor < 0) game_state->time_scale_factor = 0;
                } break;
                default: break;
            }
            event = event->next;
        }
    }
    
    dt = game_state->time_scale_factor * dt;
    
    switch(game_state->game_mode)
    {
        case Game_Mode_GAME:
        {
            UpdateAndRenderGame(game_state, controller, dt);
        } break;
        
#if 0        
        case Game_Mode_MENU_USERNAME:
        {
            UpdateAndRenderUserNameMenu(game_state, ui_context, controller);
        } break;
        case Game_Mode_MENU_MAIN:
        {
            UpdateAndRenderMainMenu(game_state, ui_context, controller);
        } break;
        case Game_Mode_MENU_WAITING_PLAYERS: 
        {
            UpdateAndRenderWaitingPlayersMenu(game_state, ui_context, controller);
        } break;
        case Game_Mode_MENU_JOIN_GAME:
        {
            UpdateAndRenderJoinSessionMenu(game_state, ui_context, controller); 
        } break;
#endif
        
    }
    
    // NOTE(fakhri): display a message if there is any
    {
        if (game_state->message_duration > 0)
        {
            game_state->message_duration -= dt;
            
            Render_PushText(&game_state->render_context, game_state->message_to_display.content, Vec3(0, 0, CentiMeter(60)), Vec4(1,1,1,1), CoordinateType_World, FontKind_Arial);
            
        }
    }
    
    // NOTE(fakhri): render the mouse cursor
    Render_PushQuad(&game_state->render_context, 
                    Vec3(os->mouse_position, 99),
                    Vec2(MiliMeter(5.f), MiliMeter(5.f)), Vec4(1, .3f, .5f, 1.f), CoordinateType_Screen);
    
    // NOTE(fakhri): Debug UI
    {
        M_Temp scratch = GetScratch(0, 0);
        
        // NOTE(fakhri): World coords axix
        {
            Render_PushQuad(&game_state->render_context, 
                            Vec3(0, 0, 0),
                            Vec2(Meter(2.0f), MiliMeter(1.0f)), Vec4(1.0f, 1.0f, 0.f, 1.f), CoordinateType_World);
            
            Render_PushQuad(&game_state->render_context, 
                            Vec3(0, 0, 0),
                            Vec2(MiliMeter(1.0f), Meter(2.0f)), Vec4(1.0f, 1.0f, 0.f, 1.f), CoordinateType_World);
        }
        
        
        // NOTE(fakhri): time scale
        {
            String8 msg = {};
            msg = PushStr8F(scratch.arena, "time scale factor: %f", game_state->time_scale_factor);
            Render_PushText(&game_state->render_context, msg, Vec3(300, 50, CentiMeter(60)), Vec4(0,0,0,1), CoordinateType_World, FontKind_Arial);
        }
        
        ReleaseScratch(scratch);
    }
    
    Render_End(&game_state->render_context);
    
    if (game_state->controller.toggle_fullscreen.pressed)
    {
        os->fullscreen ^= 1;
    }
}