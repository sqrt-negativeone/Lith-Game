
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

internal inline void
AddPlayer(Game_State *game_state, MessagePlayer *message_player, u32 player_id)
{
    Player *player = game_state->players + player_id;
    player->username = PushStr8F(os->permanent_arena, "%s", message_player->username);
    player->joined = true;
    ++game_state->players_joined_so_far;
}

internal void
HandleAvailableMessages(Game_State *game_state)
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
                AddPlayer(game_state, message_player, player_id);
            } break;
            case MessageType_From_Host_Connected_Players_List:
            {
                // NOTE(fakhri): we receive this message when we first join the host
                // the host sends a list of all the currently connected players,
                // and we are garanteed to be the last one on this list,
                // so we can safely assume that our id is `player_count - 1`
                game_state->my_player_id = message_result.message.players_count - 1;
                for(u32 player_index = 0;
                    player_index < message_result.message.players_count;
                    ++player_index)
                {
                    MessagePlayer *message_player = message_result.message.players + player_index;
                    AddPlayer(game_state, message_player, player_index);
                }
                SetFlag(game_state->flags, StateFlag_JoinedGame);
            } break;
            case MessageType_From_Host_Invalid_Username:
            {
                SetFlag(game_state->flags, StateFlag_FailedJoinGame);
            } break;
            case MessageType_From_Host_Shuffled_Deck:
            {
                AssignResidencyToPlayers(game_state);
                // NOTE(fakhri): split the deck between players
                for(u32 player_index = 0;
                    player_index < MAX_PLAYER_COUNT;
                    ++player_index)
                {
                    Player *player = game_state->players + player_index;
                    u32 card_base = player_index * CARDS_PER_PLAYER;
                    for(u32 card_offset = 0;
                        card_offset < CARDS_PER_PLAYER;
                        ++card_offset)
                    {
                        u32 compact_card_index = card_base + card_offset;
                        Card_Type card_type = UnpackCompactCardType(message_result.message.compact_deck[compact_card_index]);
                        b32 is_flipped = (player_index != game_state->my_player_id);
                        AddCardEntity(game_state, card_type, (Card_Residency)player->assigned_residency_index, is_flipped);
                    }
                }
                SetFlag(game_state->flags, StateFlag_ReceivedCards);
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
    Assert(GameMode_MENU_BEGIN < menu_mode && menu_mode < GameMode_MENU_END);
    game_state->game_mode = menu_mode;
    game_state->ui_context.selected_item = -1;
    glDepthFunc(GL_ALWAYS);
}

internal void
StartGame(Game_State *game_state)
{
    game_state->game_mode = GameMode_GAME;
    SetFlag(game_state->flags, StateFlag_WaitingForCards);
    glDepthFunc(GL_LESS);
}


internal
void UpdateAndRenderMainMenu(Game_State *game_state, UI_Context *ui_context, Controller *controller)
{
    UI_BeginFrame(ui_context, controller, &game_state->render_context);
    
    
    // NOTE(fakhri): Menu Title
    UI_Label(ui_context, Str8Lit("Lith?"), Meter(0), CentiMeter(10), FontKind_MenuTitle, CoordinateType_World);
    
    
    // NOTE(fakhri): join session button
    if (UI_Button(ui_context, Str8Lit("Join Game Room"), MiliMeter(0), MiliMeter(0), FontKind_MenuItem))
    {
#if 0
        FetchHosts(&game_state->hosts_storage);
        OpenMenu(game_state, GameMode_MENU_JOIN_GAME);
#else
        os->PushNetworkMessage(CreateConnectToServerMessage(Str8Lit("")));
        OpenMenu(game_state, GameMode_MENU_USERNAME);
#endif
    }
    
#if 0    
    // NOTE(fakhri): Host Session button
    if (UI_Button(game_state, Str8Lit("Create Game Room"), x, y, Vec2(half_screen.width, 1.1f * GetFontHeight(game_state->active_font))))
    {
        OpenMenu(game_state, GameMode_MENU_WAITING_PLAYERS);
        os->PushNetworkMessage(CreateStartHostServerMessage());
        SetFlag(game_state->session_state_flags, StateFlag_HostingGame);
    }
    y += stride;
#endif
    
    // NOTE(fakhri): Quit
    if (UI_Button(ui_context, Str8Lit("Quit"), Meter(0), -CentiMeter(10),FontKind_MenuItem))
    {
        os->quit = 1;
    }
    
    UI_EndFrame(ui_context, controller);
}

void UpdateAndRenderUserNameMenu(Game_State *game_state, UI_Context *ui_context, Controller *controller)
{
    
    UI_BeginFrame(ui_context, controller, &game_state->render_context);
    
    // NOTE(fakhri): Menu Title
    UI_Label(ui_context, Str8Lit("Connect to game"), Meter(0), CentiMeter(20), FontKind_MenuTitle, CoordinateType_World);
    
    
    // NOTE(fakhri): Username input field
    {
        UI_Label(ui_context, Str8Lit("Your username is?"), MiliMeter(0), CentiMeter(5), FontKind_MenuItem);
        
        // TODO(fakhri): fix the text not rendering in the right position
        UI_InputField(ui_context, Vec2(CentiMeter(50), CentiMeter(4)), MiliMeter(0), -CentiMeter(3),  &game_state->username_buffer, FontKind_MenuItem);
    }
    
    if (!HasFlag(game_state->flags, StateFlag_TryingJoinGame))
    {
        if (UI_Button(ui_context, Str8Lit("Join"), Meter(0), -CentiMeter(20), FontKind_MenuItem))
        {
            String8 username =  game_state->username_buffer.content;
            
            if(username.size)
            {
                os->PushNetworkMessage(CreateUsernameMessage(username));
                SetFlag(game_state->flags, StateFlag_TryingJoinGame);
            }
            else
            {
                // TODO(fakhri): dispaly error message
            }
        }
    }
    else
    {
        UI_Label(ui_context, Str8Lit("joinning the game"), Meter(0), -CentiMeter(20), FontKind_MenuItem, true);
        
        if (HasFlag(game_state->flags, StateFlag_JoinedGame))
        {
            // NOTE(fakhri): good, we wait for all players to join and then we start the game
            OpenMenu(game_state, GameMode_MENU_WAITING_PLAYERS);
            ClearFlag(game_state->flags, StateFlag_TryingJoinGame);
        }
        
        if (HasFlag(game_state->flags, StateFlag_FailedJoinGame))
        {
            // TODO(fakhri): display an error message
            ClearFlag(game_state->flags, StateFlag_TryingJoinGame);
        }
    }
    UI_EndFrame(ui_context, controller);
    
}

#if 0
internal
void UpdateAndRenderWaitingPlayersMenu(Game_State *game_state, UI_Context *ui_context, Controller *controller)
{
    Game_Session *game_state = &game_state->game_state;
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
         player_index < ArrayCount(game_state->players);
         ++player_index)
    {
        Player *player = game_state->players + player_index;
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
        if (HasFlag(game_state->flags, StateFlag_HostingGame))
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
            OpenMenu(game_state, GameMode_MENU_MAIN);
            
            if (HasFlag(game_state->flags, StateFlag_HostingGame))
            {
                // TODO(fakhri): stop the server
            }
            
            game_state->players_joined_so_far = 0;
        }
    }
    
    if (game_state->players_joined_so_far == MAX_PLAYER_COUNT)
    {
        // NOTE(fakhri): enough players have joined
        StartGame(game_state, game_state);
    }
}

void UpdateAndRenderJoinSessionMenu(Game_State *game_state, UI_Context *ui_context, Controller *controller)
{
    Game_Session *game_state = &game_state->game_state;
    
    UI_BeginFrame(ui_context, controller, &game_state->render_context);
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // NOTE(fakhri): Menu Title
    UI_Label(game_state, Str8Lit("Choose a host game"), x, y);
    f32 stride = 2.0f * GetFontHeight(game_state->active_font);
    // TODO(fakhri): draw a list of available hosts here
    b32 is_still_fetching_hosts = game_state->hosts_storage.is_fetching;
    if (!is_still_fetching_hosts)
    {
        y = 0.3f * screen.height;
        for (u32 host_index = 0;
             host_index < game_state->hosts_storage.hosts_count;
             ++host_index)
        {
            Host_Info *host_info = game_state->hosts_storage.hosts + host_index;
            UI_Label(game_state, Str8C(host_info->hostname),x, y );
            y += stride;
        }
        
        y = 0.9f * screen.height;
        
        if(UI_Button(game_state, Str8Lit("Refresh Hosts List"), x, y, Vec2(half_screen.width, 1.1f * GetFontHeight(game_state->active_font))))
        {
            FetchHosts(&game_state->hosts_storage);
        }
    }
    else
    {
        y = 0.5f * screen.height;
        UI_Label(game_state, Str8Lit("Refreshing Available Hosts"), x, y);
    }
    
    UI_EndFrame(ui_context, controller);
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
    game_state->event_buffer.arena = os->permanent_arena;
    OpenMenu(game_state, GameMode_MENU_MAIN);
    
    InitResidencies(game_state);
    game_state->time_scale_factor = 1.f;
    game_state->declared_number = Card_Number_Count;
    // @DebugOnly
#if 1
    //glDepthFunc(GL_LESS);
    AssignResidencyToPlayers(game_state);
    game_state->current_player_id = CardResidency_Down - 1;
    game_state->my_player_id      = CardResidency_Down - 1;
    AddDebugEntites(game_state);
#endif
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
    
    UI_Context *ui_context = &game_state->ui_context;
    game_state->controller = {};
    Controller *controller = &game_state->controller;
    
    HandleAvailableMessages(game_state);
    Render_Begin(&game_state->render_context, OS_FrameArena());
    
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
        case GameMode_GAME:
        {
            v4 white = Vec4(1, 1, 1,1);
            v4 red = Vec4(1, 0, 0, 1);
            
            // TODO(fakhri): render a background
            
            if(HasFlag(game_state->flags, StateFlag_WaitingForCards))
            {
                // NOTE(fakhri): we wait
                ChangeActiveFont(&game_state->render_context, FontKind_Arial);
                
                Render_PushText(&game_state->render_context, Str8Lit("waiting for cards from server"), Vec3(0, 0, 60), Vec4(1,0,1,1), CoordinateType_World, FontKind_Arial);
                
                if(HasFlag(game_state->flags, StateFlag_ReceivedCards))
                {
                    ClearFlag(game_state->flags, StateFlag_WaitingForCards);
                }
            }
            else
            {
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
                            if (entity->residency == CardResidency_None)
                            {
                                // NOTE(fakhri): ignore
                                continue;
                            }
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
                            Entity *entity_to_follow = game_state->entities + entity->entity_index_to_follow;
                            Assert(entity_to_follow);
                            if (entity_to_follow->residency == CardResidency_None)
                            {
                                // NOTE(fakhri): ignore
                                continue;
                            }
                            
                            UpdateCompanionEntity(game_state, entity, dt);
                            Render_PushImage(&game_state->render_context, entity->texture, Vec3(entity->center_pos.xy, entity_to_follow->center_pos.z + 0.01f), entity->current_dimension, CoordinateType_World,
                                             entity_to_follow->y_angle);
                            
                        } break;
                        default:
                        {
                            StopExecution;
                        } break;
                    }
                }
                
                // NOTE(fakhri): see if any residency needs reorganization
                {
                    b32 should_burn_cards = false;
                    for (u32 residency_index = 0;
                         residency_index < CardResidency_Count;
                         ++residency_index)
                    {
                        Residency *residency = game_state->residencies + residency_index;
                        if (HasFlag(residency->flags, ResidencyFlags_NeedsReorganizing))
                        {
                            ReorganizeResidencyCards(game_state, (Card_Residency)residency_index);
                            if (HasFlag(residency->flags, ResidencyFlags_Burnable))
                            {
                                // TODO(fakhri): card_number_frequencies can be a field in the Residency
                                // this way we don't have to recompute it each time we call this function
                                // but then the complexity of maintaining this frequency table
                                // will leak to change residency for example
                                // NOTE(fakhri): count the frequency of each card number in the residency
                                u32 card_number_freq[Card_Number_Count] = {};
                                for(u32 entity_index_in_residency = 0;
                                    entity_index_in_residency< residency->entity_count;
                                    ++entity_index_in_residency)
                                {
                                    u32 entity_index = residency->entity_indices[entity_index_in_residency];
                                    Entity *card_entity = game_state->entities + entity_index;
                                    ++card_number_freq[card_entity->card_type.number];
                                }
                                
                                // NOTE(fakhri): mark the cards to be removed
                                for(u32 entity_index_in_residency = 0;
                                    entity_index_in_residency< residency->entity_count;
                                    ++entity_index_in_residency)
                                {
                                    u32 entity_index = residency->entity_indices[entity_index_in_residency];
                                    Entity *card_entity = game_state->entities + entity_index;
                                    if(card_number_freq[card_entity->card_type.number] >= THRESHOLD_FOR_BURNING)
                                    {
                                        card_entity->marked_for_burning = true;
                                        should_burn_cards = true;
                                    }
                                }
                            }
                        }
                    }
                    
                    if (should_burn_cards)
                    {
                        GameEvent_PushEvent_Delay(&game_state->event_buffer, Seconds(0.5f));
                        GameEvent_PushEvent_DisplayMessag(&game_state->event_buffer, Str8Lit("Burning Cards With Too Much Duplicates"),
                                                          Vec4(1, 1, 1, 1), Vec3(0, 0, 0), CoordinateType_World,  Seconds(2.0f));
                        GameEvent_PushEvent_BurnCards(&game_state->event_buffer);
                        SetFlag(game_state->flags, StateFlag_ShouldBurnCards);
                    }
                }
                
                // NOTE(fakhri): declaring a card if the table was empty
                if (HasFlag(game_state->flags, StateFlag_ShouldDeclareCard))
                {
                    b32 did_choose_card = false;
                    
                    Render_PushQuad(&game_state->render_context, Vec3(0,0,0), Vec2(CentiMeter(80), CentiMeter(50)),Vec4(0.1f, 0.1f, 0.1f, 0.5f), CoordinateType_World);
                    
                    Render_PushText(&game_state->render_context, Str8Lit("Select A Card To Declare"),  Vec3(0,CentiMeter(20),0), Vec4(1.0f, 1.0f, 1.0f, 1.0f), CoordinateType_World, FontKind_Arial);
                    
                    // TODO(fakhri): think about how to present this
                    
                    Render_PushQuad(&game_state->render_context, Vec3(0, -CentiMeter(10), 0), 1.2f * Vec2(CARD_WIDTH, CARD_HEIGHT),Vec4(0.f, 0.f, 0.f, 0.5f), CoordinateType_World);
                    
                    if (did_choose_card)
                    {
                        ClearFlag(game_state->flags, StateFlag_ShouldDeclareCard);
                    }
                }
                
                // NOTE(fakhri): display the declared number
                if (game_state->declared_number < Card_Number_Count)
                {
                    // TODO(fakhri): think about how to present this
                    M_Temp scratch = GetScratch(0 ,0);
                    String8 msg = PushStr8F(scratch.arena, 
                                            "Declared Number is: ", game_state->declared_number);
                    v3 pos = Vec3(-CentiMeter(30), CentiMeter(24), 0);
                    pos.x = Render_PushText(&game_state->render_context, msg, pos, Vec4(0, 0, 0, 1), CoordinateType_World, FontKind_Arial);
                    Texture2D texture = game_state->frensh_deck.red_numbers_up[game_state->declared_number];
                    v2 dim = 0.15f * Vec2(MiliMeter(128.f), MiliMeter(203.f));
                    pos.x += 0.5f * dim.x;
                    pos.y += 0.25f * dim.y;
                    Render_PushImage(&game_state->render_context, texture, pos, dim, CoordinateType_World);
                    ReleaseScratch(scratch);
                    
                }
                // TODO(fakhri): AI stuff for debug?
            }
            
            
            // @DebugOnly
            {
                if (controller->right_mouse.pressed)
                {
                    for (u32 residency_type = CardResidency_Left;
                         residency_type <= CardResidency_Burnt;
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
        } break;
        case GameMode_MENU_MAIN:
        {
            UpdateAndRenderMainMenu(game_state, ui_context, controller);
        } break;
        
        case GameMode_MENU_USERNAME:
        {
            UpdateAndRenderUserNameMenu(game_state, ui_context, controller);
        } break;
        
#if 0        
        
        case GameMode_MENU_WAITING_PLAYERS: 
        {
            UpdateAndRenderWaitingPlayersMenu(game_state, ui_context, controller);
        } break;
        
        case GameMode_MENU_JOIN_GAME:
        {
            UpdateAndRenderJoinSessionMenu(game_state, ui_context, controller); 
        } break;
#endif
        default: NotImplemented;
    }
    
    // TODO(fakhri): do we need a separate buffer frame events? 
    // ei: events that only last for a frame and thus there isn't
    // a strong notion of order between them? right now the events 
    // are handled in a first come first served fashion, so it is 
    // not possible to hadnle events that needs to be processed this frame
    // if it was proceeded by events that require more than one frame to execute!!
    // NOTE(fakhri): handle game events
    {
        for(;;)
        {
            Game_Event *game_event = game_state->event_buffer.first;
            if (!game_event) break;
            game_event->duration -= dt;
            switch(game_event->kind )
            {
                case GameEventKind_DisplayMessage:
                {
                    Render_PushText(&game_state->render_context, game_event->string, game_event->string_position, game_event->string_color, game_event->coords_type, FontKind_Arial);
                } break;
                case GameEventKind_BurnCards:
                {
                    // NOTE(fakhri): burn the marked cards
                    Assert(CardResidency_Down - CardResidency_Left + 1  == MAX_PLAYER_COUNT);
                    for(u32 residency_index = CardResidency_Left;
                        residency_index <= CardResidency_Down;
                        ++residency_index)
                    {
                        Residency *residency = game_state->residencies + residency_index;
                        for(u32 index_in_residency = 0;
                            index_in_residency < residency->entity_count;
                            )
                        {
                            u32 entity_index = residency->entity_indices[index_in_residency];
                            Entity *entity = game_state->entities + entity_index;
                            if(entity->marked_for_burning)
                            {
                                entity->marked_for_burning = false;
                                ChangeResidency(game_state, entity_index, CardResidency_Burnt);
                            }
                            else
                            {
                                ++index_in_residency;
                            }
                        }
                    }
                    ClearFlag(game_state->flags, StateFlag_ShouldBurnCards);
                } break;
                case GameEventKind_Delay:
                {
                    // NOTE(fakhri): do nothing
                } break;
                case GameEventKind_ChangeCurrentPlayer:
                {
                    u32 new_player_id = (game_state->current_player_id + 1) % MAX_PLAYER_COUNT;
                    
                    M_Temp scratch = GetScratch(0 ,0);
                    String8 msg = PushStr8F(scratch.arena, 
                                            "Player's %d Turn!", new_player_id);
                    GameEvent_PushEvent_DisplayMessag(&game_state->event_buffer, msg, 
                                                      Vec4(0, 1, 1, 1), Vec3(0, CentiMeter(10), 0), CoordinateType_World, 
                                                      Seconds(1));
                    ReleaseScratch(scratch);
                    
                    game_state->current_player_id = new_player_id;
                } break;
                case GameEventKind_OpenDeclareMenu:
                {
                    ClearFlag(game_state->flags, StateFlag_ShouldOpenDeclaringMenu);
                    SetFlag(game_state->flags, StateFlag_ShouldDeclareCard);
                    // NOTE(fakhri): move selection cards out from the none residency
                    MoveAllFromResidency(game_state, CardResidency_None, CardResidency_CardSelecting);
                    
                } break;
                case GameEventKind_CloseDeclareMenu:
                {
                    SetFlag(game_state->flags, StateFlag_ShouldDeclareCard);
                    // NOTE(fakhri): move selection cards out from the none residency
                    MoveAllFromResidency(game_state, CardResidency_CardSelecting, CardResidency_None);
                    
                    MoveAllFromResidency(game_state, CardResidency_CardDeclaring, CardResidency_None);
                    GameEvent_PushEvent_ChangeCurrentPlayer(&game_state->event_buffer);
                    game_state->declared_number = game_event->declared_number;
                    ClearFlag(game_state->flags, StateFlag_ShouldDeclareCard);
                } break;
                default: NotImplemented;
            }
            
            if (game_event->duration > 0)
            {
                break;
            }
            
            GameEvent_EatFirtEvent(&game_state->event_buffer);
        }
    }
    
    // NOTE(fakhri): render the mouse cursor
    Render_PushQuad(&game_state->render_context, 
                    Vec3(os->mouse_position, 99),
                    Vec2(MiliMeter(5.f), MiliMeter(5.f)), Vec4(1, .3f, .5f, 1.f), CoordinateType_Screen);
    
    Render_End(&game_state->render_context);
    
    if (game_state->controller.toggle_fullscreen.pressed)
    {
        os->fullscreen ^= 1;
    }
}