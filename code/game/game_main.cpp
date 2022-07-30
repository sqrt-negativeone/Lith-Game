
#define TEST_NETWORKING 1

////////////////////////////////
//~ NOTE(fakhri): headers

#include "base/base_inc.h"
#include "game_ids.h"
#include "network/network_inc.h"
#include "os/os_inc.h"
#include "game/game_inc.h"

////////////////////////////////
//~ NOTE(fakhri): implementations
#include "base/base_inc.cpp"
#include "network/network_inc.cpp"
#include "os/os_inc.cpp"
#include "game_inc.cpp"

global Thread_Ctx game_tctx;

internal void
ChangeCurrentPlayer(Game_State *game_state, PlayerID next_player_id)
{
    Assert(next_player_id < game_state->players_joined_so_far);
    M_Temp scratch = GetScratch(0 ,0);
    String8 msg = PushStr8F(scratch.arena, 
                            "Player %.s's Turn!", game_state->players[next_player_id].username);
    GameCommand_PushCommand_DisplayMessag(&game_state->command_buffer, msg, 
                                          Vec4(0, 1, 1, 1), Vec3(0, CentiMeter(10), 0), CoordinateType_World, 
                                          Seconds(1));
    ReleaseScratch(scratch);
    
    game_state->prev_player_id = game_state->curr_player_id;
    game_state->curr_player_id = next_player_id;
}

internal void
PlaySelectedCards(Game_State *game_state)
{
    SetFlag(game_state->flags, StateFlag_PlayedCardThisFrame);
    game_state->prev_played_cards_count = game_state->selection_count;
    game_state->selection_count = 0;
    MoveAllFromResidency(game_state, ResidencyKind_SelectedCards, ResidencyKind_Table);
    ClearFlag(game_state->flags, StateFlag_ShouldDeclareCard);
}

internal inline void
AddPlayer(Game_State *game_state, String8 player_username, u32 player_id)
{
    Player *player = game_state->players + player_id;
    player->username = PushStr8Copy(os->permanent_arena, player_username);
    player->joined = true;
    ++game_state->players_joined_so_far;
}

internal void
PlayPlayedCards(Game_State *game_state, PlayCardMove player_move)
{
    ResidencyKind curr_player_residency = game_state->players[game_state->curr_player_id].assigned_residency_kind;
    
    if (IsResidencyEmpty(game_state, ResidencyKind_Table))
    {
        Assert(player_move.declared_number < Card_Number_Count);
        if (player_move.declared_number < Card_Number_Count)
        {
            game_state->declared_number = player_move.declared_number;
        }
    }
    
    for (u32 index = 0;
         index < player_move.played_cards_count;
         ++index)
    {
        Card_Type card_type = UnpackCompactCardType(player_move.actual_cards[index]);
        EntityID entity_id = game_state->entity_id_from_card_type[card_type.category][card_type.number];
        Entity *entity = game_state->entities + entity_id;
        if (curr_player_residency == entity->residency)
        {
            ChangeResidency(game_state, entity_id, ResidencyKind_Table);
        }
    }
    
    game_state->prev_played_cards_count = player_move.played_cards_count;
}

internal void
QuestionPreviousPlayerCredibility(Game_State *game_state)
{
    // NOTE(fakhri): if any of the previously played cards are
    // different from the declared cards then punish the prev player,
    // else punish the current player
    b32 prev_player_lied = false;
    Residency *residency = game_state->residencies + ResidencyKind_Table;
    for (u32 index = 0;
         index < game_state->prev_played_cards_count;
         ++index)
    {
        EntityID entity_id = residency->entity_ids[residency->entity_count - 1 - index];
        if (game_state->entities[entity_id].card_type.number != game_state->declared_number)
        {
            prev_player_lied = true;
            break;
        }
    } 
    
    // NOTE(fakhri): clear any card the current player has selected
    {
        ResidencyIterator iter = MakeResidencyIterator(game_state, 
                                                       GetResidencyOfCurrentPlayer(game_state));
        for(EachValidResidencyEntityID(entity_id, iter))
        {
            Entity *entity = game_state->entities + entity_id;
            ClearFlag(entity->flags, EntityFlag_Selected);
        }
    }
    
    PlayerID player_id_to_punish = prev_player_lied?
        game_state->prev_player_id : game_state->curr_player_id;
    Player *player_to_punish = game_state->players + player_id_to_punish;
    MoveAllFromResidency(game_state, ResidencyKind_Table, player_to_punish->assigned_residency_kind);
    game_state->declared_number = InvalidCardNumber;
    game_state->prev_played_cards_count = 0;
    
}

internal void
HandleAvailableMessages(Game_State *game_state)
{
    while(!os->IsHostMessageQueueEmpty())
    {
        Message *message = os->BeginHostMessageQueueRead();
        switch(message->type)
        {
            case NetworkMessage_HostDown:
            {
                if (os->IsGameHostRunning())
                {
                    os->StopGameHost();
                }
                
                if (HasFlag(game_state->flags, StateFlag_GameStarted))
                {
                    ClearFlag(game_state->flags, StateFlag_GameStarted);
                    SetFlag(game_state->flags, StateFlag_HostDown);
                }
            } break;
            case HostMessage_HostShuttingDown:
            {
                if (os->IsGameHostRunning())
                {
                    os->StopGameHost();
                }
                
                if (HasFlag(game_state->flags, StateFlag_GameStarted))
                {
                    ClearFlag(game_state->flags, StateFlag_GameStarted);
                    SetFlag(game_state->flags, StateFlag_HostDown);
                    if (message->host_closing_reason == ClosingReason_PlayerDisconnected)
                    {
                        SetFlag(game_state->flags, StateFlag_PlayerDisconnected);
                    }
                }
            } break;
            case NetworkMessage_FailedToHost:
            {
                SetFlag(game_state->flags, StateFlag_CantHost);
            } break;
            case NetworkMessage_FailedToJoin:
            {
                SetFlag(game_state->flags, StateFlag_FailedJoinGame);
            } break;
            case NetworkMessage_JoinedGame:
            {
                SetFlag(game_state->flags, StateFlag_JoinedGame);
            } break;
            case NetworkMessage_GameID:
            {
                if (!game_state->game_id.str)
                {
                    game_state->game_id.str = PushArray(os->permanent_arena, u8, message->game_id.size);
                    game_state->game_id.size = message->game_id.size;
                }
                
                Assert(message->game_id.size == game_state->game_id.size);
                MemoryCopy(game_state->game_id.str, message->game_id.str, message->game_id.size);
                SetFlag(game_state->flags, StateFlag_HostingGame);
            } break;
            case HostMessage_NewPlayerJoined:
            {
                PlayerID player_id = message->new_player_id;
                String8 player_username = message->new_username;
                AddPlayer(game_state, player_username, player_id);
            } break;
            case HostMessage_ConnectedPlayersList:
            {
                // NOTE(fakhri): we receive this message when we first join the host
                // the host sends a list of all the currently connected players,
                // and we are garanteed to be the last one on this list,
                // so we can safely assume that our id is `player_count - 1`
                game_state->my_player_id = message->players_count - 1;
                for(PlayerID player_id = 0;
                    player_id < message->players_count;
                    ++player_id)
                {
                    String8 player_username = message->players_usernames[player_id];
                    AddPlayer(game_state, player_username, player_id);
                }
                SetFlag(game_state->flags, StateFlag_UsernameValid);
            } break;
            case HostMessage_InvalidUsername:
            {
                SetFlag(game_state->flags, StateFlag_UsernameInvalid);
            } break;
            case HostMessage_ShuffledDeck:
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
                        Card_Type card_type = UnpackCompactCardType(message->compact_deck[compact_card_index]);
                        AddCardEntity(game_state, card_type, player->assigned_residency_kind);
                    }
                }
                SetFlag(game_state->flags, StateFlag_ReceivedCards);
            } break;
            case HostMessage_ChangePlayerTurn:
            {
                ChangeCurrentPlayer(game_state, message->next_player_id);
            } break;
            case HostMessage_PlayCard:
            {
                if (game_state->curr_player_id != game_state->my_player_id)
                {
                    PlayPlayedCards(game_state, message->player_move);
                }
            } break;
            case HostMessage_QuestionCredibility:
            {
                if (game_state->curr_player_id != game_state->my_player_id)
                {
                    QuestionPreviousPlayerCredibility(game_state);
                }
            } break;
            default:
            {
                NotImplemented;
            }
        }
        os->EndHostMessageQueueRead();
    }
}

internal void
ResetGameState(Game_State *game_state)
{
    game_state->flags = 0;
    game_state->highest_entity_under_cursor = InvalidEntityID;
    game_state->prev_played_cards_count = 0;
    game_state->selection_count = 0;
    game_state->entity_count = 0;
    game_state->time_scale_factor = 1.f;
    game_state->selection_limit = 13;
    game_state->declared_number = InvalidCardNumber;
    
    for(u32 index = 0;
        index < ArrayCount(game_state->players);
        ++index)
    {
        game_state->players[index].joined = false;
    }
    
    UI_OpenMenu(&game_state->ui, GameMenuKind_Main);
    InitResidencies(game_state);
    AddNullEntity(game_state);
    AddCursorEntity(game_state);
    GameCommand_ClearBuffer(&game_state->command_buffer);
}

exported
APP_HotLoad(HotLoad)
{
    os = _os;
    // NOTE(fakhri): init the game thread context
    {
        game_tctx = MakeTCTX();
        SetTCTX(&game_tctx);
    }
    
    gladLoadGL ();
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
    
    game_state->ui = UI_Init(&game_state->render_context, &game_state->controller);
    game_state->command_buffer.arena = os->permanent_arena;
    
    ResetGameState(game_state);
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
    game_state->controller = {};
    Controller *controller = &game_state->controller;
    
    HandleAvailableMessages(game_state);
    Render_Begin(&game_state->render_context, OS_FrameArena());
    
    Render_PushClear(&game_state->render_context, Vec4(0.2f, 0.2f, 0.2f, 1.f));
    
    // NOTE(fakhri): Debug UI
    if (0)
    {
        M_Temp scratch = GetScratch(0, 0);
        
        // NOTE(fakhri): World coords axix
        {
            Render_PushQuad(&game_state->render_context, 
                            Vec3(0, 0, -MAX_Z),
                            Vec2(Meter(2.0f), MiliMeter(1.0f)), Vec4(1.0f, 1.0f, 0.f, 1.f), CoordinateType_World);
            
            Render_PushQuad(&game_state->render_context, 
                            Vec3(0, 0, -MAX_Z),
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
    v4 white = Vec4(1, 1, 1,1);
    v4 red = Vec4(1, 0, 0, 1);
    
    GameMenu(game_state, dt);
    
    if (game_state->controller.right_mouse.pressed)
    {
        ResetGameState(game_state);
    }
    
    if(HasFlag(game_state->flags, StateFlag_WaitingForCards))
    {
        // NOTE(fakhri): we wait
        ChangeActiveFont(&game_state->render_context, FontKind_Arial);
        
        Render_PushText(&game_state->render_context, Str8Lit("waiting for cards from server"), Vec3(0, 0, 60), Vec4(1,0,1,1), CoordinateType_World, FontKind_Arial);
        
        if(HasFlag(game_state->flags, StateFlag_ReceivedCards))
        {
            ClearFlag(game_state->flags, StateFlag_WaitingForCards | StateFlag_ReceivedCards);
            SetFlag(game_state->flags, StateFlag_GameStarted);
            AddButtonEntity(game_state, ButtonEntityKind_QuestionCredibility, Vec3(MiliMeter(150), MiliMeter(0), 0), Vec2(MiliMeter(50), MiliMeter(30)));
            AddButtonEntity(game_state, ButtonEntityKind_PlaySelectedCards, Vec3(MiliMeter(150), -MiliMeter(40), 0), Vec2(MiliMeter(50), MiliMeter(30)));
            
            AddArrowEntity(game_state);
            
            for (Card_Number number = Card_Number_Ace;
                 number < Card_Number_Count;
                 ++number)
            {
                AddNumberEntity(game_state, number);
            }
        }
    }
    
    // NOTE(fakhri): loop over all the entities and update them
    for (u32 entity_id = 1;
         entity_id < game_state->entity_count;
         ++entity_id)
    {
        Entity *entity = game_state->entities + entity_id;
        switch(entity->type)
        {
            case EntityType_Cursor:
            {
                UpdateCursorEntity(game_state, entity);
                
                // NOTE(fakhri): render the mouse cursor
                Render_PushQuad(&game_state->render_context, 
                                entity->center_pos,
                                Vec2(MiliMeter(5.f), MiliMeter(5.f)), Vec4(1, .3f, .5f, 1.f), CoordinateType_World);
                
            } break;
            case EntityType_Card:
            {
                UpdateCardEntity(game_state, entity_id, dt);
                if (!HasFlag(entity->flags, EntityFlag_DontDrawThisFrame))
                {
                    if (HasFlag(entity->flags, EntityFlag_Selected))
                    {
                        Render_PushQuad(&game_state->render_context, 
                                        Vec3(entity->center_pos.xy, entity->center_pos.z - MiliMeter(.1f)),
                                        1.05f * entity->curr_dimension, red, CoordinateType_World, entity->orientation);
                    }
                    
                    Render_PushImage(&game_state->render_context, game_state->render_context.textures[entity->texture], entity->center_pos, entity->curr_dimension, CoordinateType_World, entity->flip_y,
                                     entity->orientation);
                    
                    v3 card_back_pos = entity->center_pos;
                    
                    v3 card_back_orientation = entity->orientation;
                    card_back_orientation.y = PI32 - card_back_orientation.y;
                    Render_PushImage(&game_state->render_context, game_state->render_context.textures[TextureID_CardBack],  entity->center_pos, entity->curr_dimension, CoordinateType_World, false, card_back_orientation);
                }
                else
                {
                    ClearFlag(entity->flags, EntityFlag_DontDrawThisFrame);
                }
            } break;
            case EntityType_Companion:
            {
                UpdateCompanionEntity(game_state, entity, dt);
                if (!HasFlag(entity->flags, EntityFlag_DontDrawThisFrame))
                {
                    Render_PushImage(&game_state->render_context, game_state->render_context.textures[entity->texture], entity->center_pos, entity->curr_dimension, CoordinateType_World,
                                     entity->flip_y, entity->orientation);
                }
                else
                {
                    ClearFlag(entity->flags, EntityFlag_DontDrawThisFrame);
                }
            } break;
            case EntityType_Button:
            {
                UpdateButtonEntity(game_state, entity, dt);
                if (!HasFlag(entity->flags, EntityFlag_DontDrawThisFrame))
                {
                    // TODO(fakhri): draw the button entity
                    v4 button_color = Vec4(251, 73, 197, 255) / 255;
                    if (entity->button_kind == ButtonEntityKind_PlaySelectedCards)
                    {
                        button_color.r = 0;
                    }
                    Render_PushQuad(&game_state->render_context, entity->center_pos, entity->curr_dimension, button_color, CoordinateType_World);
                }
                else
                {
                    ClearFlag(entity->flags, EntityFlag_DontDrawThisFrame);
                }
            } break;
            case EntityType_Numbers:
            {
                UpdateNumberEntity(game_state, entity_id, dt);
                Render_PushImage(&game_state->render_context, game_state->render_context.textures[entity->texture], entity->center_pos, entity->curr_dimension, CoordinateType_World,
                                 false, entity->orientation);
            } break;
            case EntityType_Arrow:
            {
                UpdateArrowEntity(game_state, entity, dt);
                Render_PushImage(&game_state->render_context, game_state->render_context.textures[entity->texture], entity->center_pos, entity->curr_dimension, CoordinateType_World,
                                 false, entity->orientation);
            } break;
            default:
            {
                StopExecution;
            } break;
        }
    }
    
    if (HasFlag(game_state->flags, StateFlag_HostDown))
    {
        ClearFlag(game_state->flags, StateFlag_HostDown);
        UI_OpenMenu(&game_state->ui, GameMenuKind_HostInGameError);
    }
    
    if (HasFlag(game_state->flags, StateFlag_EndGame))
    {
        ResetGameState(game_state);
    }
    
    if (HasFlag(game_state->flags, StateFlag_GameStarted))
    {
        if (HasFlag(game_state->flags, StateFlag_QuestionCredibility))
        {
            PushQuestionCredibilityNetworkMessage();
            QuestionPreviousPlayerCredibility(game_state);
        }
        
        if (HasFlag(game_state->flags, StateFlag_PlaySelectedCards))
        {
            b32 table_empty = IsResidencyEmpty(game_state, ResidencyKind_Table);
            
            ResidencyKind target_residency = table_empty? 
                ResidencyKind_SelectedCards : ResidencyKind_Table;
            ResidencyIterator iter = MakeResidencyIterator(game_state, 
                                                           GetResidencyOfCurrentPlayer(game_state));
            for(EachValidResidencyEntityID(entity_id, iter))
            {
                Entity *entity = game_state->entities + entity_id;
                if (HasFlag(entity->flags, EntityFlag_Selected))
                {
                    ClearFlag(entity->flags, EntityFlag_Selected);
                    ChangeResidency(game_state, &iter, target_residency);
                }
            }
            
            if (table_empty)
            {
                SetFlag(game_state->flags, StateFlag_ShouldOpenDeclaringMenu);
                GameCommand_PushCommand_Delay(&game_state->command_buffer, Seconds(0.2f));
                GameCommand_PushCommand_OpenDeclareMenu(&game_state->command_buffer);
                
                MoveAllFromResidency(game_state, ResidencyKind_Nonespacial, ResidencyKind_DeclarationOptions);
            }
            else
            {
                PlaySelectedCards(game_state);
            }
        }
        
        // NOTE(fakhri): see if any residency needs reorganization
        {
            b32 should_burn_cards = false;
            for (ResidencyKind residency_kind = 0;
                 residency_kind < ResidencyKind_Count;
                 ++residency_kind)
            {
                Residency *residency = game_state->residencies + residency_kind;
                if (HasFlag(residency->flags, ResidencyFlags_NeedsReorganizing))
                {
                    ReorganizeResidencyCards(game_state, (ResidencyKind)residency_kind);
                    if (HasFlag(residency->flags, ResidencyFlags_Burnable))
                    {
                        // NOTE(fakhri): count the frequency of each card number in the residency
                        u32 card_number_freq[Card_Number_Count] = {};
                        for(u32 entity_id_in_residency = 0;
                            entity_id_in_residency< residency->entity_count;
                            ++entity_id_in_residency)
                        {
                            u32 entity_id = residency->entity_ids[entity_id_in_residency];
                            Entity *card_entity = game_state->entities + entity_id;
                            ++card_number_freq[card_entity->card_type.number];
                        }
                        
                        // NOTE(fakhri): mark the cards to be removed
                        for(u32 entity_id_in_residency = 0;
                            entity_id_in_residency< residency->entity_count;
                            ++entity_id_in_residency)
                        {
                            u32 entity_id = residency->entity_ids[entity_id_in_residency];
                            Entity *card_entity = game_state->entities + entity_id;
                            if(card_number_freq[card_entity->card_type.number] >= THRESHOLD_FOR_BURNING)
                            {
                                SetFlag(card_entity->flags, EntityFlag_MarkedForBurning);
                                should_burn_cards = true;
                            }
                        }
                    }
                }
            }
            
            if (should_burn_cards)
            {
                GameCommand_PushCommand_Delay(&game_state->command_buffer, Seconds(0.5f));
                GameCommand_PushCommand_DisplayMessag(&game_state->command_buffer, Str8Lit("Burning Cards With Too Much Duplicates"),
                                                      Vec4(1, 1, 1, 1), Vec3(0, 0, 0), CoordinateType_World,  Seconds(2.0f));
                GameCommand_PushCommand_BurnCards(&game_state->command_buffer);
                SetFlag(game_state->flags, StateFlag_ShouldBurnCards);
            }
        }
        
        // NOTE(fakhri): declaring a card if the table was empty
        if (HasFlag(game_state->flags, StateFlag_ShouldDeclareCard))
        {
            b32 did_choose_card = false;
            
            Render_PushQuad(&game_state->render_context, Vec3(0,0, Meter(1)), Vec2(CentiMeter(80), CentiMeter(50)),Vec4(0.1f, 0.1f, 0.1f, 0.5f), CoordinateType_World);
            
            Render_PushText(&game_state->render_context, Str8Lit("Cards you are going to Play"),  Vec3(0,CentiMeter(15), Meter(1)), Vec4(1.0f, 1.0f, 1.0f, 1.0f), CoordinateType_World, FontKind_Arial);
            
            Render_PushText(&game_state->render_context, Str8Lit("Select A Card To Declare"),  Vec3(0,CentiMeter(20),Meter(1)), Vec4(1.0f, 1.0f, 1.0f, 1.0f), CoordinateType_World, FontKind_Arial);
            
            // TODO(fakhri): think about how to present this
        }
        
#if 1
        // NOTE(fakhri): display the declared number
        if (game_state->declared_number < Card_Number_Count)
        {
            // TODO(fakhri): think about how to present this
            M_Temp scratch = GetScratch(0 ,0);
            String8 msg = PushStr8F(scratch.arena, 
                                    "Declared Rank is: ", game_state->declared_number);
            v3 pos = Vec3(-CentiMeter(30), CentiMeter(24), MAX_Z);
            pos.x = Render_PushText(&game_state->render_context, msg, pos, Vec4(0, 0, 0, 1), CoordinateType_World, FontKind_Arial);
            Texture2D texture = game_state->render_context.textures[TextureID_BlackNumbers_Ace + game_state->declared_number];
            v2 dim = 0.15f * Vec2(MiliMeter(128.f), MiliMeter(203.f));
            pos.x += 0.5f * dim.x;
            pos.y += 0.25f * dim.y;
            Render_PushImage(&game_state->render_context, texture, pos, dim, CoordinateType_World);
            ReleaseScratch(scratch);
            
        }
#endif
        
        // TODO(fakhri): AI stuff for debug?
    }
    
    // NOTE(fakhri): command buffer
    {
        for(;;)
        {
            Game_Command *game_command = game_state->command_buffer.first;
            if (!game_command) break;
            game_command->duration -= dt;
            switch(game_command->kind)
            {
                case GameCommandKind_DisplayMessage:
                {
                    Render_PushText(&game_state->render_context, game_command->string, game_command->string_position, game_command->string_color, game_command->coords_type, FontKind_Arial);
                } break;
                case GameCommandKind_BurnCards:
                {
                    // NOTE(fakhri): burn the marked cards
                    Assert(ResidencyKind_Down - ResidencyKind_Left + 1  == MAX_PLAYER_COUNT);
                    for(EachControllableResidencyKind(residency_kind))
                    {
                        ResidencyIterator iter = MakeResidencyIterator(game_state, residency_kind);
                        for(EachValidResidencyEntityID(entity_id, iter))
                        {
                            Entity *entity = game_state->entities + entity_id;
                            if(HasFlag(entity->flags, EntityFlag_MarkedForBurning))
                            {
                                ClearFlag(entity->flags, EntityFlag_MarkedForBurning);
                                ChangeResidency(game_state, &iter, ResidencyKind_Burnt);
                            }
                        }
                        ClearFlag(game_state->flags, StateFlag_ShouldBurnCards);
                    }
                } break;
                case GameCommandKind_Delay:
                {
                    // NOTE(fakhri): do nothing
                } break;
                case GameCommandKind_OpenDeclareMenu:
                {
                    ClearFlag(game_state->flags, StateFlag_ShouldOpenDeclaringMenu);
                    SetFlag(game_state->flags, StateFlag_ShouldDeclareCard);
                    // NOTE(fakhri): move selection cards out from the none residency
                    //MoveAllFromResidency(game_state, ResidencyKind_Nil, ResidencyKind_DeclarationOptions);
                } break;
                default: NotImplemented;
            }
            
            if (game_command->duration > 0)
            {
                break;
            }
            
            GameCommand_EatFirstCommand(&game_state->command_buffer);
        }
    }
    
    Render_End(&game_state->render_context);
    
    if (HasFlag(game_state->flags, StateFlag_PlayedCardThisFrame))
    {
        PushPlayedCardNetworkMessage(game_state);
        ClearFlag(game_state->flags, StateFlag_PlayedCardThisFrame);
    }
    
    if (game_state->controller.toggle_fullscreen.pressed)
    {
        os->fullscreen ^= 1;
    }
    ClearFlag(game_state->flags, GameStateFlagsGroup_ValidOnlyForOneFrame);
}