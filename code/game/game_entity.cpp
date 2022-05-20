
internal u32
AddEntity(Game_State *game_state)
{
    Assert(game_state->entity_count < ArrayCount(game_state->entities));
    u32 entity_id = game_state->entity_count++;
    return entity_id;
}

internal void
AddNullEntity(Game_State *game_state)
{
    u32 entity_id = AddEntity(game_state);
    Entity *entity = game_state->entities + entity_id;
    entity->type = EntityType_Null_Entity;
}

internal void
AddCursorEntity(Game_State *game_state)
{
    u32 entity_id = AddEntity(game_state);
    Entity *entity = game_state->entities + entity_id;
    entity->type = EntityType_Cursor_Entity;
}

internal void
AddNumberEntity(Game_State *game_state, Card_Number number)
{
    u32 entity_id = AddEntity(game_state);
    Entity *entity = game_state->entities + entity_id;
    entity->type = EntityType_Numbers;
    entity->card_type.number = number;
    
    entity->target_dimension = 3 * Vec2(MiliMeter(6.5f), MiliMeter(10));
    entity->curr_dimension   = 3 * Vec2(MiliMeter(6.5f), MiliMeter(10));
    entity->dDimension = 20.f;
    
    entity->texture = game_state->frensh_deck.black_numbers_up[number];
    
    AddToResidency(game_state, entity_id, ResidencyKind_Nonespacial);
}

internal void
AddCompanionEntity(Game_State *game_state, Texture2D texture, v2 companion_dimensions, u32 entity_id_to_follow, v2 followed_offset)
{
    u32 companion_entity_id = AddEntity(game_state);
    Entity *companion_entity = game_state->entities + companion_entity_id;
    Entity *card_entitiy = game_state->entities + entity_id_to_follow;
    *companion_entity = {};
    companion_entity->type = EntityType_Companion;
    companion_entity->center_pos = card_entitiy->center_pos;
    companion_entity->target_pos = card_entitiy->center_pos;
    companion_entity->curr_dimension = companion_dimensions;
    companion_entity->entity_id_to_follow = entity_id_to_follow;
    companion_entity->offset_in_follwed_entity = followed_offset;
    companion_entity->texture = texture;
}

internal inline void
AddCompanion_Ace(Game_State *game_state, Texture2D category, v2 companion_size, u32 card_entity_id)
{
    AddCompanionEntity(game_state, category, companion_size, card_entity_id, Vec2(0, 0));
}

internal inline void
AddCompanion_2(Game_State *game_state, Texture2D category_up, Texture2D category_down, v2 companion_size, v2 card_dimension, u32 card_entity_id)
{
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_id, 
                       Vec2(0, MiliMeter(28.85f)));
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_id, 
                       Vec2(0, -MiliMeter(28.85f)));
}

internal inline void
AddCompanion_3(Game_State *game_state, Texture2D category_up, Texture2D category_down, v2 companion_size, v2 card_dimension, u32 card_entity_id)
{
    AddCompanion_Ace(game_state, category_up, companion_size, card_entity_id);
    
    AddCompanion_2(game_state, category_up, category_down, companion_size, card_dimension, card_entity_id);
}

internal inline void
AddCompanion_4(Game_State *game_state, Texture2D category_up, Texture2D category_down, v2 companion_size, v2 card_dimension, u32 card_entity_id)
{
    
    v2 companion_offset = Vec2(MiliMeter(11.55f), MiliMeter(28.85f));
    
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_id, 
                       Vec2(-companion_offset.x, companion_offset.y));
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_id, 
                       Vec2(companion_offset.x, companion_offset.y));
    
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_id, 
                       Vec2(-companion_offset.x, -companion_offset.y));
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_id, 
                       Vec2(companion_offset.x, -companion_offset.y));
    
}

internal inline void
AddCompanion_5(Game_State *game_state, Texture2D category_up, Texture2D category_down, v2 companion_size, v2 card_dimension, u32 card_entity_id)
{
    AddCompanion_Ace(game_state, category_up, companion_size, card_entity_id);
    
    AddCompanion_4(game_state, category_up, category_down, companion_size, card_dimension, card_entity_id);
}

internal inline  void
AddCompanion_6(Game_State *game_state, Texture2D category_up, Texture2D category_down, v2 companion_size, v2 card_dimension, u32 card_entity_id)
{
    AddCompanion_4(game_state, category_up, category_down, companion_size, card_dimension, card_entity_id);
    
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_id, 
                       Vec2(-MiliMeter(11.55f), 0));
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_id, 
                       Vec2(MiliMeter(11.55f), 0));
    
}

internal inline void
AddCompanion_7(Game_State *game_state, Texture2D category_up, Texture2D category_down, v2 companion_size, v2 card_dimension, u32 card_entity_id)
{
    AddCompanion_6(game_state, category_up, category_down, companion_size, card_dimension, card_entity_id);
    
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_id, 
                       Vec2(0, MiliMeter(14.45f)));
    
}

internal inline void
AddCompanion_8(Game_State *game_state, Texture2D category_up, Texture2D category_down, v2 companion_size, v2 card_dimension, u32 card_entity_id)
{
    AddCompanion_7(game_state, category_up, category_down, companion_size, card_dimension, card_entity_id);
    
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_id, 
                       Vec2(0, -MiliMeter(14.45f)));
    
}

internal inline void
AddCompanion_9(Game_State *game_state, Texture2D category_up, Texture2D category_down, v2 companion_size, v2 card_dimension, u32 card_entity_id)
{
    AddCompanion_Ace(game_state, category_up, companion_size, card_entity_id);
    
    AddCompanion_4(game_state, category_up, category_down, companion_size, card_dimension, card_entity_id);
    
    v2 companion_offset = Vec2(MiliMeter(11.55f), MiliMeter(9.55f));
    
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_id, 
                       Vec2(-companion_offset.x, companion_offset.y));
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_id, 
                       Vec2(companion_offset.x, companion_offset.y));
    
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_id, 
                       Vec2(-companion_offset.x, -companion_offset.y));
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_id, 
                       Vec2(companion_offset.x, -companion_offset.y));
    
}

internal inline void
AddCompanion_10(Game_State *game_state, Texture2D category_up, Texture2D category_down, v2 companion_size, v2 card_dimension, u32 card_entity_id)
{
    
    AddCompanion_4(game_state, category_up, category_down, companion_size, card_dimension, card_entity_id);
    
    v2 companion_offset = Vec2(MiliMeter(11.55f), MiliMeter(9.55f));
    
    // NOTE(fakhri): the four at the sides
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_id, 
                       Vec2(-companion_offset.x, companion_offset.y));
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_id, 
                       Vec2(companion_offset.x, companion_offset.y));
    
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_id, 
                       Vec2(-companion_offset.x, -companion_offset.y));
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_id, 
                       Vec2(companion_offset.x, -companion_offset.y));
    
    // NOTE(fakhri): middle two
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_id, 
                       Vec2(0, MiliMeter(19.15f)));
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_id, 
                       Vec2(0, -MiliMeter(19.15f)));
    
}

internal inline  void
AddCardCompanions(Game_State *game_state, Frensh_Suited_Cards_Texture *frensh_deck, Card_Type card_type, v2 card_dimension, u32 card_entity_id)
{
    Texture2D category_up = {};
    Texture2D category_down = {};
    
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
    
    Texture2D number_up = {}; 
    Texture2D number_down = {};
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
    
    v2 padding        = 0.6f * Vec2(0.5f, 0.5f);
    v2 companion_size = Vec2(MiliMeter(6.4f), MiliMeter(10));
    
    v2 companion_offset = +0.5f * card_dimension - 0.5f * companion_size - padding;
    
    
    // @Hardcoded
    AddCompanionEntity(game_state, number_up, Vec2(MiliMeter(6.5f), MiliMeter(10)),
                       card_entity_id,
                       Vec2(-MiliMeter(22.35f), MiliMeter(36.5f)));
    
    AddCompanionEntity(game_state, category_up, Vec2(MiliMeter(7.2f), MiliMeter(7.2f)),
                       card_entity_id,
                       Vec2(-MiliMeter(22.4f), MiliMeter(26.4f)));
    
    AddCompanionEntity(game_state, number_down, Vec2(MiliMeter(6.4f), MiliMeter(10)),
                       card_entity_id,
                       Vec2(MiliMeter(22.35f), -MiliMeter(36.5f)));
    AddCompanionEntity(game_state, category_down, Vec2(MiliMeter(7.2f), MiliMeter(7.2f)),
                       card_entity_id,
                       Vec2(MiliMeter(22.4f), -MiliMeter(26.4f)));
    
    
    companion_size = Vec2(MiliMeter(13.7f), MiliMeter(13.7f));
    switch (card_type.number)
    {
        case Card_Number_Ace:
        {
            AddCompanion_Ace(game_state, category_up, companion_size, card_entity_id);
        } break;
        case Card_Number_2:
        {
            AddCompanion_2(game_state, category_up, category_down, companion_size, card_dimension, card_entity_id);
        } break;
        case Card_Number_3:
        {
            AddCompanion_3(game_state, category_up, category_down, companion_size, card_dimension, card_entity_id);
        } break;
        case Card_Number_4:
        {
            AddCompanion_4(game_state, category_up, category_down, companion_size, card_dimension, card_entity_id);
        } break;
        case Card_Number_5:
        {
            AddCompanion_5(game_state, category_up, category_down, companion_size, card_dimension, card_entity_id);
        } break;
        case Card_Number_6:
        {
            AddCompanion_6(game_state, category_up, category_down, companion_size, card_dimension, card_entity_id);
        } break;
        case Card_Number_7:
        {
            AddCompanion_7(game_state, category_up, category_down, companion_size, card_dimension, card_entity_id);
        } break;
        case Card_Number_8:
        {
            AddCompanion_8(game_state, category_up, category_down, companion_size, card_dimension, card_entity_id);
            
        } break;
        case Card_Number_9:
        {
            AddCompanion_9(game_state, category_up, category_down, companion_size, card_dimension, card_entity_id);
        } break;
        case Card_Number_10:
        {
            AddCompanion_10(game_state, category_up, category_down, companion_size, card_dimension, card_entity_id);
        } break;
        case Card_Number_Jack:
        {
            Texture2D jack = frensh_deck->jacks[card_type.category];
            
            AddCompanion_Ace(game_state, jack, Vec2(MiliMeter(36.9f), MiliMeter(62.0f)), card_entity_id);
        } break;
        case Card_Number_Queen:
        {
            Texture2D queen = frensh_deck->queens[card_type.category];
            
            AddCompanion_Ace(game_state, queen, Vec2(MiliMeter(36.9f), MiliMeter(62.0f)), card_entity_id);
        } break;
        case Card_Number_King:
        {
            Texture2D king = frensh_deck->kings[card_type.category];
            
            AddCompanion_Ace(game_state, king, Vec2(MiliMeter(36.9f), MiliMeter(62.0f)), card_entity_id);
        } break;
    }
}


internal void
AddCardEntity(Game_State *game_state, Card_Type card_type, ResidencyKind card_residency, b32 is_fliped = false)
{
    u32 card_entity_id = AddEntity(game_state);
    Entity *card = game_state->entities + card_entity_id;
    *card = {};
    card->type = EntityType_Card;
    card->card_type = card_type;
    card->target_dimension   = Vec2(CARD_WIDTH, CARD_HEIGHT);
    card->curr_dimension  = Vec2(CARD_WIDTH, CARD_HEIGHT);
    AddToResidency(game_state, card_entity_id, card_residency);
    
    card->dy_angle = 4 * PI32;
    
    if (is_fliped)
    {
        card->target_y_angle = PI32;
        card->y_angle = PI32;
    }
    
#if TEST_ONE_CARD
    card->target_pos.xy = Vec2(0, 0);
    card->residency_pos.xy = card->center_pos.xy;
#endif
    
    card->dDimension = 20.f;
    
    Frensh_Suited_Cards_Texture *frensh_deck = &game_state->frensh_deck;
    card->texture = frensh_deck->card_frame_texture;
    
    AddCardCompanions(game_state, frensh_deck, card_type, card->curr_dimension, card_entity_id);
} 


internal void
MoveEntity(Game_State *game_state, Entity *entity, f32 spring_constant, f32 friction, f32 mass, f32 dt)
{
    if (entity->entity_id_to_follow)
    {
        Entity *followed_entity = game_state->entities + entity->entity_id_to_follow;
        f32 following_trigger_distance = 0.0f;
        if (LengthSquaredVec2(followed_entity->center_pos.xy - entity->center_pos.xy) > following_trigger_distance)
        {
            entity->target_pos.xy = followed_entity->center_pos.xy + entity->offset_in_follwed_entity;
        }
    }
    
    v2 acceleration = -spring_constant * (entity->center_pos.xy - entity->target_pos.xy) - friction * entity->velocity;
    acceleration *= 1.0f / mass;
    entity->velocity += dt * acceleration;
    entity->center_pos.xy += dt * entity->velocity + 0.5f * Square(dt) * acceleration;
    entity->center_pos.z = entity->target_pos.z;
}

internal void
UpdateCursorEntity(Game_State *game_state, Entity *cursor_entity)
{
    cursor_entity->center_pos = WorldCoordsFromScreenCoords(&game_state->render_context, os->mouse_position);
    
    f32 max_z = -1;
    game_state->highest_entity_under_cursor = 0;
    for (EachValidResidencyKind(residency_kind))
    {
        // NOTE(fakhri): find the card entity with the highest z value in residency
        ResidencyIterator iter = MakeResidencyIterator(game_state, residency_kind);
        for(EachValidResidencyEntityID(entity_id, iter))
        {
            Entity *entity = game_state->entities + entity_id;
            if (IsInsideRect(RectCentDim(entity->center_pos.xy, entity->curr_dimension), cursor_entity->center_pos.xy) && 
                max_z < entity->center_pos.z)
            {
                game_state->highest_entity_under_cursor = entity_id;
                max_z = entity->center_pos.z;
            }
        }
    }
}

internal void
UpdateCardEntity(Game_State *game_state, EntityID entity_id, f32 dt)
{
    Entity *entity = game_state->entities + entity_id;
    Entity *cursor_entity = game_state->entities + (u32)EntityType_Cursor_Entity;
    Assert(cursor_entity->type == EntityType_Cursor_Entity);
    Assert(ResidencyKind_Nil < entity->residency && entity->residency < ResidencyKind_Count);
    
    if (HasFlag(entity->flags, EntityFlag_UnderCursor) && 
        !IsInsideRect(RectCentDim(entity->center_pos.xy, entity->curr_dimension),
                      cursor_entity->center_pos.xy))
    {
        ClearFlag(entity->flags, EntityFlag_UnderCursor);
        entity->target_dimension = Vec2(CARD_WIDTH, CARD_HEIGHT);
    }
    
    if (game_state->highest_entity_under_cursor == entity_id)
    {
        // NOTE(fakhri): we are under the cursor
        if (!HasFlag(entity->flags, EntityFlag_UnderCursor))
        {
            SetFlag(entity->flags, EntityFlag_UnderCursor);
            entity->target_dimension = 1.1f * Vec2(CARD_WIDTH, CARD_HEIGHT);
        }
        
        if (game_state->controller.left_mouse.pressed)
        {
            Player *curr_player = game_state->players + game_state->curr_player_id;
            if (entity->residency == curr_player->assigned_residency_kind)
            {
                if (HasFlag(entity->flags, EntityFlag_Selected))
                {
                    ClearFlag(entity->flags, EntityFlag_Selected);
                    game_state->selection_count -= 1;
                    entity->target_pos.y -= MiliMeter(10);
                }
                else
                {
                    if (game_state->selection_count < game_state->selection_limit)
                    {
                        ++game_state->selection_count;
                        SetFlag(entity->flags, EntityFlag_Selected);
                        entity->target_pos.y += MiliMeter(10);
                    }
                    else
                    {
                        M_Temp scratch = GetScratch(0, 0);
                        String8 msg = PushStr8F(scratch.arena, "you can't select more than %d cards", game_state->selection_limit);
                        GameCommand_PushCommand_DisplayMessag(&game_state->command_buffer, msg, Vec4(1, 1, 1, 1), Vec3(0, 0, 50), CoordinateType_World, Seconds(1.0f));
                        ReleaseScratch(scratch);
                    }
                }
            }
            else
            {
                GameCommand_PushCommand_DisplayMessag(&game_state->command_buffer, Str8Lit("you can't select other players cards!"), Vec4(1, 1, 1, 1), Vec3(0, 0, 50), CoordinateType_World, Seconds(2.0f));
            }
        }
    }
    
    entity->y_angle = MoveTowards(entity->y_angle, entity->target_y_angle, entity->dy_angle * dt);
    
    // NOTE(fakhri): update dimension
    entity->curr_dimension = Vec2MoveTowards(entity->curr_dimension,
                                             entity->target_dimension,
                                             dt * entity->dDimension);
    
    MoveEntity(game_state, entity, 100, 10, 0.5f, dt);
}

internal void PlaySelectedCards(Game_State *game_state);
internal void
UpdateNumberEntity(Game_State *game_state, EntityID entity_id, f32 dt)
{
    Entity *entity = game_state->entities + entity_id;
    Entity *cursor_entity = game_state->entities + (u32)EntityType_Cursor_Entity;
    MoveEntity(game_state, entity, 100, 10, 0.5f, dt);
    
    if (HasFlag(entity->flags, EntityFlag_UnderCursor) && 
        !IsInsideRect(RectCentDim(entity->center_pos.xy, entity->curr_dimension),
                      cursor_entity->center_pos.xy))
    {
        ClearFlag(entity->flags, EntityFlag_UnderCursor);
        entity->target_dimension = 3 * Vec2(MiliMeter(6.5f), MiliMeter(10));
    }
    
    if (game_state->highest_entity_under_cursor == entity_id)
    {
        // NOTE(fakhri): we are under the cursor
        if (!HasFlag(entity->flags, EntityFlag_UnderCursor))
        {
            SetFlag(entity->flags, EntityFlag_UnderCursor);
            entity->target_dimension = 1.1f * 3 * Vec2(MiliMeter(6.5f), MiliMeter(10));
        }
        
        if (game_state->controller.left_mouse.pressed)
        {
            game_state->declared_number = entity->card_type.number;
            PlaySelectedCards(game_state);
            MoveAllFromResidency(game_state, ResidencyKind_DeclarationOptions, ResidencyKind_Nonespacial);
        }
    }
    
    // NOTE(fakhri): update dimension
    entity->curr_dimension = Vec2MoveTowards(entity->curr_dimension,
                                             entity->target_dimension,
                                             dt * entity->dDimension);
    
}

internal void
UpdateCompanionEntity(Game_State *game_state, Entity *entity, f32 dt)
{
    Assert(entity->entity_id_to_follow);
    
    MoveEntity(game_state, entity, 85.f, 0.5f, 0.1f, dt);
    
    Entity *entity_to_follow = game_state->entities + entity->entity_id_to_follow;
    Assert(entity_to_follow);
    entity->center_pos.z =  entity_to_follow->center_pos.z + 0.01f;
    entity->y_angle = entity_to_follow->y_angle;
    
    if (entity_to_follow->residency == ResidencyKind_Nil)
    {
        SetFlag(entity->flags, EntityFlag_DontDrawThisFrame);
    }
}

internal void ChangeCurrentPlayer(Game_State *game_state);
internal void
UpdateButtonEntity(Game_State *game_state, Entity *entity, f32 dt)
{
    Entity *cursor_entity = game_state->entities + (u32)EntityType_Cursor_Entity;
    
    b32 clicked = 0;
    b32 mouse_inside = IsInsideRect(RectCentDim(entity->center_pos.xy, entity->curr_dimension),
                                    cursor_entity->center_pos.xy);
    if (mouse_inside)
    {
        if (game_state->controller.left_mouse.pressed)
        {
            SetFlag(entity->flags, EntityFlag_Pressed);
            entity->target_dimension = 1.2f * Vec2(MiliMeter(50), MiliMeter(30));
            clicked = true;
        }
    }
    
    if (HasFlag(entity->flags, EntityFlag_Pressed))
    {
        if (game_state->controller.left_mouse.released)
        {
            entity->target_dimension = Vec2(MiliMeter(50), MiliMeter(30));
            ClearFlag(entity->flags, EntityFlag_Pressed);
        }
    }
    
    // NOTE(fakhri): update dimension
    entity->curr_dimension = Vec2MoveTowards(entity->curr_dimension,
                                             entity->target_dimension,
                                             dt * entity->dDimension);
    
    
    if (clicked)
    {
        switch(entity->button_kind)
        {
            case ButtonEntityKind_PlaySelectedCards:
            {
                if (game_state->selection_count)
                {
                    SetFlag(game_state->flags, StateFlag_PlaySelectedCards);
                }
                else
                {
                    GameCommand_PushCommand_DisplayMessag(&game_state->command_buffer, Str8Lit("Select a card first"), Vec4(0, 0, 0, 1), Vec3(0, 0, 50), CoordinateType_World, Seconds(1.0f));
                }
            } break;
            case ButtonEntityKind_QuestionCredibility:
            {
                if (game_state->prev_played_cards_count)
                {
                    // NOTE(fakhri): if any of the previously played cards are
                    // different from the declared cards then punish the prev player,
                    // else punish the current player
                    b32 prev_player_lied = false;
                    for (u32 index = 0;
                         index < game_state->prev_played_cards_count;
                         ++index)
                    {
                        Residency *residency = game_state->residencies + ResidencyKind_Table;
                        EntityID entity_id = residency->entity_ids[residency->entity_count - 1 - index];
                        if (game_state->entities[entity_id].card_type.number != game_state->declared_number)
                        {
                            prev_player_lied = true;
                            break;
                        }
                    } 
                    
                    PlayerID player_id_to_punish = prev_player_lied?
                        game_state->prev_player_id : game_state->curr_player_id;
                    Player *player_to_punish = game_state->players + player_id_to_punish;
                    MoveAllFromResidency(game_state, ResidencyKind_Table, player_to_punish->assigned_residency_kind);
                    ChangeCurrentPlayer(game_state);
                    // TODO(fakhri): clear the declared number
                }
            } break;
            default: NotImplemented;
        }
    }
}

internal void
AddButtonEntity(Game_State *game_state, ButtonEntityKind button_kind, v3 center_pos, v2 dimension)
{
    u32 button_entity_id = AddEntity(game_state);
    Entity *button = game_state->entities + button_entity_id;
    MemoryZero(button, sizeof(Entity));
    
    button->type             = EntityType_Button;
    button->button_kind      = button_kind;
    button->center_pos       = center_pos;
    button->target_dimension = dimension;
    button->curr_dimension   = dimension;
    button->dDimension       = 20.f;
}

internal void
AddDebugEntites(Game_State *game_state)
{
    
    for(ResidencyKind residency_kind = ResidencyKind_Nil;
        residency_kind < ResidencyKind_Count;
        ++residency_kind)
    {
        MemoryZeroArray(game_state->residencies[residency_kind].entity_ids);
    }
    
    game_state->entity_count = 0;
    for (u32 residency_index = 0;
         residency_index < ArrayCount(game_state->residencies);
         ++residency_index)
    {
        game_state->residencies[residency_index].entity_count = 0;
    }
    
    AddNullEntity(game_state);
    AddCursorEntity(game_state);
    
    
    game_state->game_mode = GameMode_GAME;
    SetFlag(game_state->flags, StateFlag_ReceivedCards);
    
#if TEST_ONE_CARD
    AddCardEntity(game_state, MakeCardType(Category_Hearts, Card_Number_Jack), ResidencyKind_Down);
#else
    
    for (u32 player_index = 0;
         player_index < MAX_PLAYER_COUNT;
         ++player_index)
    {
        Player *player = game_state->players + player_index;
        player->joined = true;
        player->assigned_residency_kind = ResidencyKind_Left + player_index;
        player->username = PushStr8F(os->permanent_arena, "%c", 'a' + player_index);
    }
    
    for (u32 card_index = 0;
         card_index < 13;
         ++card_index)
    {
        AddCardEntity(game_state, MakeCardType(Category_Hearts, (Card_Number)card_index), ResidencyKind_Left);
    }
    
    
    for (u32 card_index = 0;
         card_index < 13;
         ++card_index)
    {
        AddCardEntity(game_state, MakeCardType(Category_Tiles, (Card_Number)card_index), ResidencyKind_Right);
    }
    
    for (u32 card_index = 0;
         card_index < 13;
         ++card_index)
    {
        AddCardEntity(game_state, MakeCardType(Category_Clovers, (Card_Number)card_index), ResidencyKind_Up);
    }
    
    for (u32 card_index = 0;
         card_index < 13;
         ++card_index)
    {
        AddCardEntity(game_state, MakeCardType(Category_Pikes, (Card_Number)card_index), ResidencyKind_Down);
    }
    
#if 0    
    for (u32 card_index = 0;
         card_index < 13;
         ++card_index)
    {
        AddCardEntity(game_state, MakeCardType(Category_Pikes, (Card_Number)card_index), ResidencyKind_DeclarationOptions);
    }
#endif
    
    for (Card_Number number = Card_Number_Ace;
         number < Card_Number_Count;
         ++number)
    {
        AddNumberEntity(game_state, number);
    }
    
    AddButtonEntity(game_state, ButtonEntityKind_QuestionCredibility, Vec3(MiliMeter(150), MiliMeter(0), 0), Vec2(MiliMeter(50), MiliMeter(30)));
    AddButtonEntity(game_state, ButtonEntityKind_PlaySelectedCards, Vec3(MiliMeter(150), -MiliMeter(40), 0), Vec2(MiliMeter(50), MiliMeter(30)));
#endif
}
