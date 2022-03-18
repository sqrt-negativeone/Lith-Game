
internal void
InitResidencies(Game_State *game_state)
{
    // NOTE(fakhri): up residency
    {
        Residency *residency = game_state->residencies + Card_Residency_Up;
        residency->is_horizonal = true;
        residency->is_stacked = false;
        residency->controlling_player_id = MAX_PLAYER_COUNT;
    }
    
    // NOTE(fakhri): down residency
    {
        Residency *residency = game_state->residencies + Card_Residency_Down;
        residency->is_horizonal = true;
        residency->is_stacked = false;
        residency->controlling_player_id = MAX_PLAYER_COUNT;
    }
    
    // NOTE(fakhri): left residency
    {
        Residency *residency = game_state->residencies + Card_Residency_Left;
        residency->is_horizonal = false;
        residency->is_stacked = false;
        residency->controlling_player_id = MAX_PLAYER_COUNT;
    }
    
    // NOTE(fakhri): right residency
    {
        Residency *residency = game_state->residencies + Card_Residency_Right;
        residency->is_horizonal = false;
        residency->is_stacked = false;
        residency->controlling_player_id = MAX_PLAYER_COUNT;
    }
    
    // NOTE(fakhri): table residency
    {
        Residency *residency = game_state->residencies + Card_Residency_Table;
        residency->is_horizonal = false;
        residency->is_stacked = true;
        residency->controlling_player_id = MAX_PLAYER_COUNT;
    }
    
    // NOTE(fakhri): burnt residency
    {
        Residency *residency = game_state->residencies + Card_Residency_Burnt;
        residency->is_horizonal = false;
        residency->is_stacked = true;
        residency->controlling_player_id = MAX_PLAYER_COUNT;
    }
    
}

internal v2
FindFirstPositionInResidencyRow(Game_State *game_state, Card_Residency residency_type, u32 entity_count)
{
    v2 result = {};
    f32 world_width = MAX_UNITS_PER_X;
    f32 world_height = MAX_UNITS_PER_X * game_state->rendering_context.aspect_ratio;
    
    
    switch(residency_type)
    {
        case Card_Residency_Up:
        {
            result.x = 0.5f * (CARD_WIDTH + world_width - entity_count * CARD_WIDTH - (entity_count - 1) * CARD_X_GAP);
            result.y = world_height - 6.5f;
        } break;
        case Card_Residency_Down:
        {
            result.y = 6.5f;
            result.x = 0.5f * (CARD_WIDTH + world_width - entity_count * CARD_WIDTH - (entity_count - 1) * CARD_X_GAP);
        } break;
        case Card_Residency_Left:
        {
            result.x = 6.5f;
            result.y = 0.5f * (CARD_HEIGHT + world_height - entity_count * CARD_HEIGHT - (entity_count - 1) * CARD_Y_GAP);
        } break;
        case Card_Residency_Right:
        {
            result.x = world_width - 6.5f;
            result.y = 0.5f * (CARD_HEIGHT + world_height - entity_count * CARD_HEIGHT - (entity_count - 1) * CARD_Y_GAP);
        } break;
        case Card_Residency_Table:
        {
            // TODO(fakhri): think about how we will position the cards in this residency
            result = vec2(0.45f * world_width, 0.55f * world_height);
        } break;
        
        case Card_Residency_Burnt:
        {
            result = vec2(0.55f * world_width, 0.55f * world_height);
        } break;
        
        default:
        {
            // NOTE(fakhri): not yet implemented
            BreakDebugger();
        } break;
    }
    return result;
}

internal void
ReorganizeResidencyCards(Game_State *game_state, Card_Residency residency_type)
{
    // TODO(fakhri): think about having a limit on how many cards there can be on one row
    // and move to the next row if we exceed that max number
    if (residency_type != Card_Residency_None)
    {
        Residency *residency = game_state->residencies + residency_type;
        u32 remaining_cards = residency->entity_count;
        
#define MAX_CARDS_PER_RESIDENCY_ROW 17
        u32 cards_per_row_remaining = MAX_CARDS_PER_RESIDENCY_ROW;
        
        v3 residency_pos = {};
        residency_pos.xy = FindFirstPositionInResidencyRow(game_state, 
                                                           residency_type, 
                                                           Min(cards_per_row_remaining, remaining_cards));
        
        for (u32 entity_index_in_residency = 0;
             entity_index_in_residency< residency->entity_count;
             ++entity_index_in_residency)
        {
            u32 entity_index = residency->entity_indices[entity_index_in_residency];
            Entity *entity = game_state->entities + entity_index;
            
            entity->residency_pos = residency_pos;
            if (!entity->followed_entity_index)
            {
                // NOTE(fakhri): go to the residency position if i'm not already following an entitiy
                entity->target_pos = entity->residency_pos;
            }
            
            --cards_per_row_remaining;
            --remaining_cards;
            // NOTE(fakhri): update the residency position for the next card
            
            if(residency->is_horizonal)
            {
                if(cards_per_row_remaining)
                {
                    residency_pos.x += CARD_WIDTH + CARD_X_GAP;
                }
                else
                {
                    cards_per_row_remaining = MAX_CARDS_PER_RESIDENCY_ROW;
                    f32 old_y = residency_pos.y;
                    residency_pos.xy = FindFirstPositionInResidencyRow(game_state,
                                                                       residency_type, 
                                                                       Min(cards_per_row_remaining, remaining_cards));
                    residency_pos.y = old_y + CARD_HEIGHT;
                }
            }
            else
            {
                // NOTE(fakhri): veritcal residency
                if(residency->is_stacked)
                {
                    residency_pos.y += 0.1f;
                }
                else
                {
                    if (cards_per_row_remaining)
                    {
                        residency_pos.y += CARD_HEIGHT + CARD_Y_GAP;
                    }
                    else
                    {
                        cards_per_row_remaining = MAX_CARDS_PER_RESIDENCY_ROW;
                        f32 old_x = residency_pos.x;
                        residency_pos.xy = FindFirstPositionInResidencyRow(game_state,
                                                                           residency_type, 
                                                                           Min(cards_per_row_remaining, remaining_cards));
                        residency_pos.x = old_x + CARD_WIDTH + CARD_X_GAP;
                    }
                }
            }
            residency_pos.z += 0.3f;
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
            Residency *entity_residency = game_state->residencies + old_residency;
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
        Residency *entity_residency = game_state->residencies + residency;
        Assert(entity_residency->entity_count < ArrayCount(entity_residency->entity_indices));
        entity_residency->entity_indices[entity_residency->entity_count++] = entity_index;
        entity->residency = residency;
        
        ReorganizeResidencyCards(game_state, old_residency);
        ReorganizeResidencyCards(game_state, residency);
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
    companion_entity->type = Entity_Type_Companion;
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
    card->type = Entity_Type_Card;
    card->card_type = card_type;
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
    
    // TODO(fakhri): make sure thatonly one card should be able to get selected under cursor
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


internal void
BurnExtraCardEntities(Game_State *game_state, Residency *residency, Residency *burnt_residency)
{
    // NOTE(fakhri): count the frequency of each card
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
        
        if(card_number_freq[card_entity->card_type.number] == Category_Count)
        {
            // TODO(fakhri): benchmark this, it can be optimazed if it turned out it is slowing us
            // we can just mark the cards to be removed and remove them later in a single pass
            // like we did in the server, but probably the number cards in each residency is so small
            // that we won't care?
            ChangeResidency(game_state, entity_index, Card_Residency_Burnt);
            card_entity->target_y_angle = 0;
        }
    }
}


internal void
AddDebugEntites(Game_State *game_state)
{
    game_state->entity_count = 0;
    for (u32 residency_index = 0;
         residency_index < ArrayCount(game_state->residencies);
         ++residency_index)
    {
        game_state->residencies[residency_index].entity_count = 0;
    }
    
    AddNullEntity(game_state);
    AddCursorEntity(game_state);
    
#if TEST_ONE_CARD
    AddCardEntity(game_state, MakeCardType(Category_Hearts, Card_Number_10), Card_Residency_Down);
#else
    game_state->game_mode = Game_Mode_GAME;
    SetFlag(game_state->game_session.flags, SESSION_FLAG_HOST_FINISHED_SPLITTING_DECK);
    for (u32 player_index = 0;
         player_index < MAX_PLAYER_COUNT;
         ++player_index)
    {
        Player *player = game_state->game_session.players + player_index;
        player->joined = true;
        player->assigned_residency = (Card_Residency)(player_index + 1);
        player->username = PushStringF(&os->permanent_arena, "%s", "a");
    }
    
    for (u32 card_index = 0;
         card_index < 1;
         ++card_index)
    {
        AddCardEntity(game_state, MakeCardType(Category_Tiles, (Card_Number)card_index), Card_Residency_Up);
    }
    
    for (u32 card_index = 1;
         card_index < 13;
         ++card_index)
    {
        AddCardEntity(game_state, MakeCardType(Category_Tiles, (Card_Number)card_index), Card_Residency_Down);
    }
    
    for (u32 card_index = 0;
         card_index < 1;
         ++card_index)
    {
        AddCardEntity(game_state, MakeCardType(Category_Tiles, (Card_Number)card_index), Card_Residency_Left);
    }
    
    for (u32 card_index = 1;
         card_index < 13;
         ++card_index)
    {
        AddCardEntity(game_state, MakeCardType(Category_Tiles, (Card_Number)card_index), Card_Residency_Down);
    }
    
    for (u32 card_index = 0;
         card_index < 1;
         ++card_index)
    {
        AddCardEntity(game_state, MakeCardType(Category_Tiles, (Card_Number)card_index), Card_Residency_Right);
    }
    
    for (u32 card_index = 1;
         card_index < 13;
         ++card_index)
    {
        AddCardEntity(game_state, MakeCardType(Category_Tiles, (Card_Number)card_index), Card_Residency_Down);
    }
    
    for (u32 card_index = 0;
         card_index < 13;
         ++card_index)
    {
        AddCardEntity(game_state, MakeCardType(Category_Pikes, (Card_Number)card_index), Card_Residency_Down);
    }
    
    Residency *burnt_residency = game_state->residencies + Card_Residency_Burnt;
    
    for(u32 residency_index = Card_Residency_Left;
        residency_index <= Card_Residency_Down;
        ++residency_index)
    {
        Residency *residency = game_state->residencies + residency_index;
        BurnExtraCardEntities(game_state, residency, burnt_residency);
    }
#endif
}
