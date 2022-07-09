
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
    entity->type = EntityType_Null;
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
AddArrowEntity(Game_State *game_state)
{
    u32 entity_id = AddEntity(game_state);
    Entity *entity = game_state->entities + entity_id;
    entity->type = EntityType_Arrow;
    entity->texture = TextureID_Arrow;
    entity->residency = ResidencyKind_Nil;
    entity->curr_dimension = Vec2(MiliMeter(30), MiliMeter(30));
    entity->d_orientation.z = 4 * PI32;
    
}

internal void
AddCursorEntity(Game_State *game_state)
{
    u32 entity_id = AddEntity(game_state);
    Entity *entity = game_state->entities + entity_id;
    entity->type = EntityType_Cursor;
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
    
    entity->texture = TextureID_BlackNumbers_Ace + number;
    
    AddToResidency(game_state, entity_id, ResidencyKind_Nonespacial);
}

internal void
AddCompanionEntity(Game_State *game_state, TextureID texture, b32 flip_y, v2 companion_dimensions, u32 entity_id_to_follow, v2 followed_offset)
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
    companion_entity->flip_y = flip_y;
}

internal inline void
AddCompanion_Ace(Game_State *game_state, TextureID category, v2 companion_size, u32 card_entity_id)
{
    AddCompanionEntity(game_state, category, false, companion_size, card_entity_id, Vec2(0, 0));
}

internal inline void
AddCompanion_2(Game_State *game_state, TextureID category, v2 companion_size, v2 card_dimension, u32 card_entity_id)
{
    AddCompanionEntity(game_state, category, false, companion_size, card_entity_id, 
                       Vec2(0, MiliMeter(28.85f)));
    AddCompanionEntity(game_state, category, true, companion_size, card_entity_id, 
                       Vec2(0, -MiliMeter(28.85f)));
}

internal inline void
AddCompanion_3(Game_State *game_state, TextureID category, v2 companion_size, v2 card_dimension, u32 card_entity_id)
{
    AddCompanion_Ace(game_state, category, companion_size, card_entity_id);
    
    AddCompanion_2(game_state, category, companion_size, card_dimension, card_entity_id);
}

internal inline void
AddCompanion_4(Game_State *game_state, TextureID category, v2 companion_size, v2 card_dimension, u32 card_entity_id)
{
    v2 companion_offset = Vec2(MiliMeter(11.55f), MiliMeter(28.85f));
    
    AddCompanionEntity(game_state, category, false, companion_size, card_entity_id, 
                       Vec2(-companion_offset.x, companion_offset.y));
    AddCompanionEntity(game_state, category, false, companion_size, card_entity_id, 
                       Vec2(companion_offset.x, companion_offset.y));
    
    AddCompanionEntity(game_state, category, true, companion_size, card_entity_id, 
                       Vec2(-companion_offset.x, -companion_offset.y));
    AddCompanionEntity(game_state, category, true, companion_size, card_entity_id, 
                       Vec2(companion_offset.x, -companion_offset.y));
}

internal inline void
AddCompanion_5(Game_State *game_state, TextureID category, v2 companion_size, v2 card_dimension, u32 card_entity_id)
{
    AddCompanion_Ace(game_state, category, companion_size, card_entity_id);
    
    AddCompanion_4(game_state, category, companion_size, card_dimension, card_entity_id);
}

internal inline  void
AddCompanion_6(Game_State *game_state, TextureID category, v2 companion_size, v2 card_dimension, u32 card_entity_id)
{
    AddCompanion_4(game_state, category, companion_size, card_dimension, card_entity_id);
    
    AddCompanionEntity(game_state, category, false, companion_size, card_entity_id, 
                       Vec2(-MiliMeter(11.55f), 0));
    AddCompanionEntity(game_state, category, false, companion_size, card_entity_id, 
                       Vec2(MiliMeter(11.55f), 0));
    
}

internal inline void
AddCompanion_7(Game_State *game_state, TextureID category, v2 companion_size, v2 card_dimension, u32 card_entity_id)
{
    AddCompanion_6(game_state, category, companion_size, card_dimension, card_entity_id);
    
    AddCompanionEntity(game_state, category, false, companion_size, card_entity_id, 
                       Vec2(0, MiliMeter(14.45f)));
    
}

internal inline void
AddCompanion_8(Game_State *game_state, TextureID category, v2 companion_size, v2 card_dimension, u32 card_entity_id)
{
    AddCompanion_7(game_state, category, companion_size, card_dimension, card_entity_id);
    
    AddCompanionEntity(game_state, category, true, companion_size, card_entity_id, 
                       Vec2(0, -MiliMeter(14.45f)));
    
}

internal inline void
AddCompanion_9(Game_State *game_state, TextureID category, v2 companion_size, v2 card_dimension, u32 card_entity_id)
{
    AddCompanion_Ace(game_state, category, companion_size, card_entity_id);
    
    AddCompanion_4(game_state, category, companion_size, card_dimension, card_entity_id);
    
    v2 companion_offset = Vec2(MiliMeter(11.55f), MiliMeter(9.55f));
    
    AddCompanionEntity(game_state, category, false, companion_size, card_entity_id, 
                       Vec2(-companion_offset.x, companion_offset.y));
    AddCompanionEntity(game_state, category, false, companion_size, card_entity_id, 
                       Vec2(companion_offset.x, companion_offset.y));
    
    AddCompanionEntity(game_state, category, true, companion_size, card_entity_id, 
                       Vec2(-companion_offset.x, -companion_offset.y));
    AddCompanionEntity(game_state, category, true, companion_size, card_entity_id, 
                       Vec2(companion_offset.x, -companion_offset.y));
    
}

internal inline void
AddCompanion_10(Game_State *game_state, TextureID category, v2 companion_size, v2 card_dimension, u32 card_entity_id)
{
    
    AddCompanion_4(game_state, category, companion_size, card_dimension, card_entity_id);
    
    v2 companion_offset = Vec2(MiliMeter(11.55f), MiliMeter(9.55f));
    
    // NOTE(fakhri): the four at the sides
    AddCompanionEntity(game_state, category, false, companion_size, card_entity_id, 
                       Vec2(-companion_offset.x, companion_offset.y));
    AddCompanionEntity(game_state, category, false, companion_size, card_entity_id, 
                       Vec2(companion_offset.x, companion_offset.y));
    
    AddCompanionEntity(game_state, category, true, companion_size, card_entity_id, 
                       Vec2(-companion_offset.x, -companion_offset.y));
    AddCompanionEntity(game_state, category, true, companion_size, card_entity_id, 
                       Vec2(companion_offset.x, -companion_offset.y));
    
    // NOTE(fakhri): middle two
    AddCompanionEntity(game_state, category, false, companion_size, card_entity_id, 
                       Vec2(0, MiliMeter(19.15f)));
    AddCompanionEntity(game_state, category, true, companion_size, card_entity_id, 
                       Vec2(0, -MiliMeter(19.15f)));
    
}

internal inline  void
AddCardCompanions(Game_State *game_state, Card_Type card_type, v2 card_dimension, u32 card_entity_id)
{
    TextureID category_texture = TextureID_None;
    b32 is_black = false;
    switch(card_type.category)
    {
        case Category_Clovers: // black
        {
            is_black = true;
            category_texture = TextureID_Clover;
        } break;
        case Category_Hearts:  // red
        {
            is_black = false;
            category_texture = TextureID_Heart;
        } break;
        case Category_Tiles:   // red
        {
            is_black = false;
            category_texture = TextureID_Tiles;
        } break;
        case Category_Pikes:   // black
        {
            is_black = true;
            category_texture = TextureID_Pikes;
        } break;
        default: NotImplemented;
    }
    
    TextureID number_texture;
    
    number_texture = (is_black? TextureID_BlackNumbers_Ace: TextureID_RedNumbers_Ace)
        + card_type.number;
    
    v2 padding        = 0.6f * Vec2(0.5f, 0.5f);
    v2 companion_size = Vec2(MiliMeter(6.4f), MiliMeter(10));
    
    v2 companion_offset = +0.5f * card_dimension - 0.5f * companion_size - padding;
    
    
    // @Hardcoded
    AddCompanionEntity(game_state, number_texture, false, Vec2(MiliMeter(6.5f), MiliMeter(10)),
                       card_entity_id,
                       Vec2(-MiliMeter(22.35f), MiliMeter(36.5f)));
    
    AddCompanionEntity(game_state, category_texture, false, Vec2(MiliMeter(7.2f), MiliMeter(7.2f)),
                       card_entity_id,
                       Vec2(-MiliMeter(22.4f), MiliMeter(26.4f)));
    
    AddCompanionEntity(game_state, number_texture, true, Vec2(MiliMeter(6.4f), MiliMeter(10)),
                       card_entity_id,
                       Vec2(MiliMeter(22.35f), -MiliMeter(36.5f)));
    AddCompanionEntity(game_state, category_texture, true, Vec2(MiliMeter(7.2f), MiliMeter(7.2f)),
                       card_entity_id,
                       Vec2(MiliMeter(22.4f), -MiliMeter(26.4f)));
    
    
    companion_size = Vec2(MiliMeter(13.7f), MiliMeter(13.7f));
    switch (card_type.number)
    {
        case Card_Number_Ace:
        {
            AddCompanion_Ace(game_state, category_texture, companion_size, card_entity_id);
        } break;
        case Card_Number_2:
        {
            AddCompanion_2(game_state, category_texture, companion_size, card_dimension, card_entity_id);
        } break;
        case Card_Number_3:
        {
            AddCompanion_3(game_state, category_texture, companion_size, card_dimension, card_entity_id);
        } break;
        case Card_Number_4:
        {
            AddCompanion_4(game_state, category_texture, companion_size, card_dimension, card_entity_id);
        } break;
        case Card_Number_5:
        {
            AddCompanion_5(game_state, category_texture, companion_size, card_dimension, card_entity_id);
        } break;
        case Card_Number_6:
        {
            AddCompanion_6(game_state, category_texture, companion_size, card_dimension, card_entity_id);
        } break;
        case Card_Number_7:
        {
            AddCompanion_7(game_state, category_texture, companion_size, card_dimension, card_entity_id);
        } break;
        case Card_Number_8:
        {
            AddCompanion_8(game_state, category_texture, companion_size, card_dimension, card_entity_id);
            
        } break;
        case Card_Number_9:
        {
            AddCompanion_9(game_state, category_texture, companion_size, card_dimension, card_entity_id);
        } break;
        case Card_Number_10:
        {
            AddCompanion_10(game_state, category_texture, companion_size, card_dimension, card_entity_id);
        } break;
        case Card_Number_Jack:
        {
            AddCompanion_Ace(game_state, TextureID_Jack + card_type.category, Vec2(MiliMeter(36.9f), MiliMeter(62.0f)), card_entity_id);
        } break;
        case Card_Number_Queen:
        {
            AddCompanion_Ace(game_state, TextureID_Queen + card_type.category, Vec2(MiliMeter(36.9f), MiliMeter(62.0f)), card_entity_id);
        } break;
        case Card_Number_King:
        {
            AddCompanion_Ace(game_state, TextureID_King + card_type.category, Vec2(MiliMeter(36.9f), MiliMeter(62.0f)), card_entity_id);
        } break;
    }
}


internal void
AddCardEntity(Game_State *game_state, Card_Type card_type, ResidencyKind card_residency)
{
    u32 card_entity_id = AddEntity(game_state);
    Entity *card = game_state->entities + card_entity_id;
    *card = {};
    card->type = EntityType_Card;
    card->card_type = card_type;
    game_state->card_type_to_entity_id_map[card_type.category][card_type.number] = card_entity_id;
    card->target_dimension   = Vec2(CARD_WIDTH, CARD_HEIGHT);
    card->curr_dimension  = Vec2(CARD_WIDTH, CARD_HEIGHT);
    AddToResidency(game_state, card_entity_id, card_residency);
    
    card->d_orientation.y = 4 * PI32;
    card->d_orientation.z = 4 * PI32;
    
#if TEST_ONE_CARD
    card->target_pos.xy = Vec2(0, 0);
    card->residency_pos.xy = card->center_pos.xy;
#endif
    
    card->dDimension = 20.f;
    
    card->texture = TextureID_CardFrame;
    
    AddCardCompanions(game_state, card_type, card->curr_dimension, card_entity_id);
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
    cursor_entity->center_pos.z = MAX_Z;
    
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
    Entity *cursor_entity = game_state->entities + (u32)EntityType_Cursor;
    Assert(cursor_entity->type == EntityType_Cursor);
    Assert(ResidencyKind_Nil < entity->residency && entity->residency < ResidencyKind_Count);
    
    if (HasFlag(entity->flags, EntityFlag_UnderCursor) && game_state->highest_entity_under_cursor != entity_id)
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
            if ((game_state->curr_player_id == game_state->my_player_id) && (entity->residency == curr_player->assigned_residency_kind))
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
    
    entity->orientation = Vec3MoveTowards(entity->orientation, entity->target_orientation, entity->d_orientation * dt);
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
    Entity *cursor_entity = game_state->entities + (u32)EntityType_Cursor;
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
        
        if (HasFlag(game_state->flags, StateFlag_ShouldDeclareCard))
        {
            if (game_state->controller.left_mouse.pressed)
            {
                game_state->declared_number = entity->card_type.number;
                PlaySelectedCards(game_state);
                MoveAllFromResidency(game_state, ResidencyKind_DeclarationOptions, ResidencyKind_Nonespacial);
            }
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
    
    Assert(IsValidEntityID(entity->entity_id_to_follow) && entity->entity_id_to_follow < game_state->entity_count);
    Entity *entity_to_follow = game_state->entities + entity->entity_id_to_follow;
    entity->orientation.y = entity_to_follow->orientation.y;
    entity->target_pos.z =  entity_to_follow->center_pos.z + MiliMeter(0.1f);
    MoveEntity(game_state, entity, 85.f, 0.5f, 0.1f, dt);
    
    
    if (entity_to_follow->residency == ResidencyKind_Nil)
    {
        SetFlag(entity->flags, EntityFlag_DontDrawThisFrame);
    }
}

internal void
UpdateButtonEntity(Game_State *game_state, Entity *entity, f32 dt)
{
    Entity *cursor_entity = game_state->entities + (u32)EntityType_Cursor;
    
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
    
    
    if ((game_state->curr_player_id == game_state->my_player_id) && clicked)
    {
        switch(entity->button_kind)
        {
            case ButtonEntityKind_PlaySelectedCards:
            {
                if (game_state->selection_count)
                {
                    // NOTE(fakhri): defer handling the button click
                    SetFlag(game_state->flags, StateFlag_PlaySelectedCards);
                }
                else
                {
                    GameCommand_PushCommand_DisplayMessag(&game_state->command_buffer, Str8Lit("Select a card first"), Vec4(0, 0, 0, 1), Vec3(0, 0, 50), CoordinateType_World, Seconds(1.0f));
                }
            } break;
            case ButtonEntityKind_QuestionCredibility:
            {
                Residency *residency = game_state->residencies + ResidencyKind_Table;
                if (game_state->prev_played_cards_count)
                {
                    // NOTE(fakhri): defer handling the button click
                    SetFlag(game_state->flags, StateFlag_QuestionCredibility);
                }
                else
                {
                    GameCommand_PushCommand_DisplayMessag(&game_state->command_buffer, Str8Lit("Table empty"),
                                                          Vec4(1, 1, 1, 1), Vec3(0, 0, MAX_Z), CoordinateType_World,  Seconds(2.0f));
                }
            } break;
            default: NotImplemented;
        }
    }
}

internal void
UpdateArrowEntity(Game_State *game_state, Entity *entity, f32 dt)
{
    ResidencyKind curr_player_residency = game_state->players[game_state->curr_player_id].assigned_residency_kind;
    
    if (curr_player_residency != entity->residency)
    {
        entity->residency = curr_player_residency;
        switch(entity->residency)
        {
            case ResidencyKind_Down:
            {
                entity->target_pos = Vec3(0, -CentiMeter(10), 0);
                entity->target_orientation.z = PI32 / 2;
            } break;
            case ResidencyKind_Up:
            {
                entity->target_pos = Vec3(0, +CentiMeter(10), 0);
                entity->target_orientation.z = -PI32 / 2;
            } break;
            case ResidencyKind_Left:
            {
                entity->target_pos = Vec3(-CentiMeter(10), 0, 0);
                entity->target_orientation.z = PI32;
            } break;
            case ResidencyKind_Right:
            {
                entity->target_pos = Vec3(CentiMeter(10), 0, 0);
                entity->target_orientation.z = 0;
            } break;
        }
    }
    entity->orientation = Vec3MoveTowards(entity->orientation, entity->target_orientation, entity->d_orientation * dt);
    MoveEntity(game_state, entity, 100, 10, 0.5f, dt);
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
    AddArrowEntity(game_state);
    
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
