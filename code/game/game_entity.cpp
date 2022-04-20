
internal void
InitResidencies(Game_State *game_state)
{
    // NOTE(fakhri): up residency
    {
        Residency *residency = game_state->residencies + Card_Residency_Up;
        residency->is_horizonal = true;
        residency->is_stacked = false;
        residency->controlling_player_id = InvalidePlayerID;
    }
    
    // NOTE(fakhri): down residency
    {
        Residency *residency = game_state->residencies + Card_Residency_Down;
        residency->is_horizonal = true;
        residency->is_stacked = false;
        residency->controlling_player_id = InvalidePlayerID;
    }
    
    // NOTE(fakhri): left residency
    {
        Residency *residency = game_state->residencies + Card_Residency_Left;
        residency->is_horizonal = false;
        residency->is_stacked = false;
        residency->controlling_player_id = InvalidePlayerID;
    }
    
    // NOTE(fakhri): right residency
    {
        Residency *residency = game_state->residencies + Card_Residency_Right;
        residency->is_horizonal = false;
        residency->is_stacked = false;
        residency->controlling_player_id = InvalidePlayerID;
    }
    
    // NOTE(fakhri): table residency
    {
        Residency *residency = game_state->residencies + Card_Residency_Table;
        residency->is_horizonal = false;
        residency->is_stacked = true;
        residency->controlling_player_id = InvalidePlayerID;
    }
    
    // NOTE(fakhri): burnt residency
    {
        Residency *residency = game_state->residencies + Card_Residency_Burnt;
        residency->is_horizonal = false;
        residency->is_stacked = true;
        residency->controlling_player_id = InvalidePlayerID;
    }
    
}

internal inline void
DisplayMessageForDuration(Game_State *game_state, String8 message, f32 duration_in_seconds)
{
    CopyStringToBuffer(&game_state->message_to_display, message);
    game_state->message_duration = duration_in_seconds;
}

internal inline void
StopMessageDisplay(Game_State *game_state)
{
    game_state->message_duration = 0;
}

internal void
BurnExtraCardEntities(Game_State *game_state, Card_Residency card_residency)
{
    Residency *residency = game_state->residencies + card_residency;
    Assert(residency->controlling_player_id != InvalidePlayerID);
    
    // TODO(fakhri): card_number_frequencies can be a field in the Residency
    // this way we don't have to recompute it each time we call this function
    // NOTE(fakhri): count the frequency of each card number
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
            card_entity->marked_for_burning = true;
            game_state->should_burn_cards = true;
        }
    }
    
    if(game_state->should_burn_cards)
    {
        // NOTE(fakhri): we don't burn the cards right away, we rather wait a 
        // little bit and then burn them.
        game_state->time_to_burn_cards = Seconds(1.f);
        DisplayMessageForDuration(game_state, Str8Lit("Burning Extra cards"), Seconds(2));
    }
}

internal v2
FindFirstPositionInResidencyRow(Game_State *game_state, Card_Residency residency_type, u32 entity_count)
{
    v2 result = {};
    
    v2 start_point = Vec2(CentiMeter(40), CentiMeter(20));
    switch(residency_type)
    {
        case Card_Residency_Up:
        {
            result.x = -0.5f * (entity_count * CARD_WIDTH + (entity_count - 1) * CARD_HORIZONTAL_GAP);
            result.y = start_point.y;
        } break;
        case Card_Residency_Down:
        {
            result.x = -0.5f * (entity_count * CARD_WIDTH + (entity_count - 1) * CARD_HORIZONTAL_GAP);
            result.y = -start_point.y;
        } break;
        case Card_Residency_Left:
        {
            result.x = -start_point.x;
            result.y = 0.5f * (entity_count - 1) * (CARD_HEIGHT + CARD_VIRTICAL_GAP);
        } break;
        case Card_Residency_Right:
        {
            result.x = +start_point.x;
            result.y = 0.5f * (entity_count - 1) * (CARD_HEIGHT + CARD_VIRTICAL_GAP);
        } break;
        case Card_Residency_Table:
        {
            // TODO(fakhri): think about how we will position the cards in this residency
            result = Vec2(-CentiMeter(5), 0);
        } break;
        
        case Card_Residency_Burnt:
        {
            result = Vec2(CentiMeter(5), 0);
        } break;
        
        default:
        {
            // NOTE(fakhri): not yet implemented
            StopExecution;
        } break;
    }
    return result;
}

internal void
ReorganizeResidencyCards(Game_State *game_state, Card_Residency residency_type)
{
    if (residency_type != Card_Residency_None)
    {
        Residency *residency = game_state->residencies + residency_type;
        u32 remaining_cards = residency->entity_count;
        
#define MAX_CARDS_PER_RESIDENCY_ROW 15
        u32 remaining_cards_per_row = MAX_CARDS_PER_RESIDENCY_ROW;
        
        v3 residency_pos = {};
        residency_pos.xy = FindFirstPositionInResidencyRow(game_state, 
                                                           residency_type, 
                                                           Min(remaining_cards_per_row, remaining_cards));
        
        for (u32 entity_index_in_residency = 0;
             entity_index_in_residency< residency->entity_count;
             ++entity_index_in_residency)
        {
            u32 entity_index = residency->entity_indices[entity_index_in_residency];
            Entity *entity = game_state->entities + entity_index;
            
            entity->residency_pos = residency_pos;
            if (!entity->entity_index_to_follow)
            {
                // NOTE(fakhri): go to the residency position if i'm not already following an entitiy
                entity->target_pos = entity->residency_pos;
            }
            
            --remaining_cards_per_row;
            --remaining_cards;
            
            // NOTE(fakhri): update the residency position for the next card
            if(residency->is_horizonal)
            {
                if(remaining_cards_per_row)
                {
                    residency_pos.x += CARD_HORIZONTAL_ADVANCE;
                    residency_pos.z += MiliMeter(20);
                }
                else
                {
                    remaining_cards_per_row = MAX_CARDS_PER_RESIDENCY_ROW;
                    f32 old_y = residency_pos.y;
                    residency_pos.xy = FindFirstPositionInResidencyRow(game_state,
                                                                       residency_type, 
                                                                       Min(remaining_cards_per_row, remaining_cards));
                    residency_pos.y = old_y + CARD_VIRTICAL_ADVANCE;
                    
                    residency_pos.z -= CentiMeter(20);
                }
            }
            else
            {
                // NOTE(fakhri): veritcal residency
                if(residency->is_stacked)
                {
                    residency_pos.y += MiliMeter(0.24f);
                }
                else
                {
                    if (remaining_cards_per_row)
                    {
                        residency_pos.y -= CARD_VIRTICAL_ADVANCE;
                    }
                    else
                    {
                        remaining_cards_per_row = MAX_CARDS_PER_RESIDENCY_ROW;
                        f32 old_x = residency_pos.x;
                        residency_pos.xy = FindFirstPositionInResidencyRow(game_state,
                                                                           residency_type, 
                                                                           Min(remaining_cards_per_row, remaining_cards));
                        residency_pos.x = old_x + CARD_HORIZONTAL_ADVANCE;
                    }
                }
                residency_pos.z += MiliMeter(20);
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
        
        if (entity_residency->controlling_player_id != InvalidePlayerID)
        {
            BurnExtraCardEntities(game_state, residency);
        }
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
AddCompanionEntity(Game_State *game_state, Texture2D texture, v2 companion_dimensions, u32 entity_index_to_follow, v2 followed_offset)
{
    u32 companion_entity_index = AddEntity(game_state);
    Entity *companion_entity = game_state->entities + companion_entity_index;
    Entity *card_entitiy = game_state->entities + entity_index_to_follow;
    *companion_entity = {};
    companion_entity->type = Entity_Type_Companion;
    companion_entity->center_pos = card_entitiy->center_pos;
    companion_entity->target_pos = card_entitiy->center_pos;
    companion_entity->current_dimension = companion_dimensions;
    companion_entity->entity_index_to_follow = entity_index_to_follow;
    companion_entity->offset_in_follwed_entity = followed_offset;
    companion_entity->texture = texture;
}

internal inline void
AddCompanion_Ace(Game_State *game_state, Texture2D category, v2 companion_size, u32 card_entity_index)
{
    AddCompanionEntity(game_state, category, companion_size, card_entity_index, Vec2(0, 0));
}

internal inline void
AddCompanion_2(Game_State *game_state, Texture2D category_up, Texture2D category_down, v2 companion_size, v2 card_dimension, u32 card_entity_index)
{
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_index, 
                       Vec2(0, MiliMeter(28.85f)));
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_index, 
                       Vec2(0, -MiliMeter(28.85f)));
}

internal inline void
AddCompanion_3(Game_State *game_state, Texture2D category_up, Texture2D category_down, v2 companion_size, v2 card_dimension, u32 card_entity_index)
{
    AddCompanion_Ace(game_state, category_up, companion_size, card_entity_index);
    
    AddCompanion_2(game_state, category_up, category_down, companion_size, card_dimension, card_entity_index);
}

internal inline void
AddCompanion_4(Game_State *game_state, Texture2D category_up, Texture2D category_down, v2 companion_size, v2 card_dimension, u32 card_entity_index)
{
    
    v2 companion_offset = Vec2(MiliMeter(11.55f), MiliMeter(28.85f));
    
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_index, 
                       Vec2(-companion_offset.x, companion_offset.y));
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_index, 
                       Vec2(companion_offset.x, companion_offset.y));
    
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_index, 
                       Vec2(-companion_offset.x, -companion_offset.y));
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_index, 
                       Vec2(companion_offset.x, -companion_offset.y));
    
}

internal inline void
AddCompanion_5(Game_State *game_state, Texture2D category_up, Texture2D category_down, v2 companion_size, v2 card_dimension, u32 card_entity_index)
{
    AddCompanion_Ace(game_state, category_up, companion_size, card_entity_index);
    
    AddCompanion_4(game_state, category_up, category_down, companion_size, card_dimension, card_entity_index);
}

internal inline  void
AddCompanion_6(Game_State *game_state, Texture2D category_up, Texture2D category_down, v2 companion_size, v2 card_dimension, u32 card_entity_index)
{
    AddCompanion_4(game_state, category_up, category_down, companion_size, card_dimension, card_entity_index);
    
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_index, 
                       Vec2(-MiliMeter(11.55f), 0));
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_index, 
                       Vec2(MiliMeter(11.55f), 0));
    
}

internal inline void
AddCompanion_7(Game_State *game_state, Texture2D category_up, Texture2D category_down, v2 companion_size, v2 card_dimension, u32 card_entity_index)
{
    AddCompanion_6(game_state, category_up, category_down, companion_size, card_dimension, card_entity_index);
    
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_index, 
                       Vec2(0, MiliMeter(14.45f)));
    
}

internal inline void
AddCompanion_8(Game_State *game_state, Texture2D category_up, Texture2D category_down, v2 companion_size, v2 card_dimension, u32 card_entity_index)
{
    AddCompanion_7(game_state, category_up, category_down, companion_size, card_dimension, card_entity_index);
    
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_index, 
                       Vec2(0, -MiliMeter(14.45f)));
    
}

internal inline void
AddCompanion_9(Game_State *game_state, Texture2D category_up, Texture2D category_down, v2 companion_size, v2 card_dimension, u32 card_entity_index)
{
    AddCompanion_Ace(game_state, category_up, companion_size, card_entity_index);
    
    AddCompanion_4(game_state, category_up, category_down, companion_size, card_dimension, card_entity_index);
    
    v2 companion_offset = Vec2(MiliMeter(11.55f), MiliMeter(9.55f));
    
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_index, 
                       Vec2(-companion_offset.x, companion_offset.y));
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_index, 
                       Vec2(companion_offset.x, companion_offset.y));
    
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_index, 
                       Vec2(-companion_offset.x, -companion_offset.y));
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_index, 
                       Vec2(companion_offset.x, -companion_offset.y));
    
}

internal inline void
AddCompanion_10(Game_State *game_state, Texture2D category_up, Texture2D category_down, v2 companion_size, v2 card_dimension, u32 card_entity_index)
{
    
    AddCompanion_4(game_state, category_up, category_down, companion_size, card_dimension, card_entity_index);
    
    v2 companion_offset = Vec2(MiliMeter(11.55f), MiliMeter(9.55f));
    
    // NOTE(fakhri): the four at the sides
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_index, 
                       Vec2(-companion_offset.x, companion_offset.y));
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_index, 
                       Vec2(companion_offset.x, companion_offset.y));
    
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_index, 
                       Vec2(-companion_offset.x, -companion_offset.y));
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_index, 
                       Vec2(companion_offset.x, -companion_offset.y));
    
    // NOTE(fakhri): middle two
    AddCompanionEntity(game_state, category_up, companion_size, card_entity_index, 
                       Vec2(0, MiliMeter(19.15f)));
    AddCompanionEntity(game_state, category_down, companion_size, card_entity_index, 
                       Vec2(0, -MiliMeter(19.15f)));
    
}

internal inline  void
AddCardCompanions(Game_State *game_state, Frensh_Suited_Cards_Texture *frensh_deck, Card_Type card_type, v2 card_dimension, u32 card_entity_index)
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
                       card_entity_index,
                       Vec2(-MiliMeter(22.35f), MiliMeter(36.5f)));
    
    AddCompanionEntity(game_state, category_up, Vec2(MiliMeter(7.2f), MiliMeter(7.2f)),
                       card_entity_index,
                       Vec2(-MiliMeter(22.4f), MiliMeter(26.4f)));
    
    AddCompanionEntity(game_state, number_down, Vec2(MiliMeter(6.4f), MiliMeter(10)),
                       card_entity_index,
                       Vec2(MiliMeter(22.35f), -MiliMeter(36.5f)));
    AddCompanionEntity(game_state, category_down, Vec2(MiliMeter(7.2f), MiliMeter(7.2f)),
                       card_entity_index,
                       Vec2(MiliMeter(22.4f), -MiliMeter(26.4f)));
    
    
    companion_size = Vec2(MiliMeter(13.7f), MiliMeter(13.7f));
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
            Texture2D jack = frensh_deck->jacks[card_type.category];
            
            AddCompanion_Ace(game_state, jack, Vec2(MiliMeter(36.9f), MiliMeter(62.0f)), card_entity_index);
        } break;
        case Card_Number_Queen:
        {
            Texture2D queen = frensh_deck->queens[card_type.category];
            
            AddCompanion_Ace(game_state, queen, Vec2(MiliMeter(36.9f), MiliMeter(62.0f)), card_entity_index);
        } break;
        case Card_Number_King:
        {
            Texture2D king = frensh_deck->kings[card_type.category];
            
            AddCompanion_Ace(game_state, king, Vec2(MiliMeter(36.9f), MiliMeter(62.0f)), card_entity_index);
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
    card->target_dimension   = Vec2(CARD_WIDTH, CARD_HEIGHT);
    card->current_dimension  = Vec2(CARD_WIDTH, CARD_HEIGHT);
    ChangeResidency(game_state, card_entity_index, card_residency);
    
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
    
    AddCardCompanions(game_state, frensh_deck, card_type, card->current_dimension, card_entity_index);
} 


internal void
MoveEntity(Game_State *game_state, Entity *entity, f32 spring_constant, f32 friction, f32 mass, f32 dt)
{
    if (entity->entity_index_to_follow)
    {
        Entity *followed_entity = game_state->entities + entity->entity_index_to_follow;
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
UpdateCursorEntity(Game_State *game_state, Entity *entity)
{
    entity->center_pos = WorldCoordsFromScreenCoords(&game_state->render_context, os->mouse_position);
}

internal void
UpdateCardEntity(Game_State *game_state, u32 entity_index, f32 dt)
{
    Entity *entity = game_state->entities + entity_index;
    Entity *cursor_entity = game_state->entities + (u32)Entity_Type_Cursor_Entity;
    Assert(cursor_entity->type == Entity_Type_Cursor_Entity);
    
    // TODO(fakhri): make sure thatonly one card should be able to get selected under cursor
    b32 should_be_under_cursor = IsInsideRect(RectCentDim(entity->center_pos.xy, entity->current_dimension), cursor_entity->center_pos.xy) ;
    
    if (!entity->is_under_cursor && should_be_under_cursor)
    {
        entity->is_under_cursor = true;
        entity->target_dimension = 1.1f * Vec2(CARD_WIDTH, CARD_HEIGHT);
    }
    
    if (entity->is_under_cursor && !should_be_under_cursor)
    {
        entity->is_under_cursor = false;
        entity->target_dimension = Vec2(CARD_WIDTH, CARD_HEIGHT);
    }
    
    if (entity->is_under_cursor)
    {
        if (!entity->is_pressed)
        {
            if(game_state->controller.left_mouse.pressed)
            {
                // NOTE(fakhri): make sure we are not pressing another card
                if (game_state->card_pressed_index == 0)
                {
                    entity->is_pressed = true;
                    entity->entity_index_to_follow = Entity_Type_Cursor_Entity;
                    game_state->card_pressed_index = entity_index;
                    entity->target_pos.z = 50.f;
                }
            }
        }
    }
    
    if (entity->is_pressed)
    {
        b32 can_move_to_table = false;
        // TODO(fakhri): allow the player to only play his own cards, but allow him to move any card
        // cuz why not LOL
        if (IsInsideRect(RectCentDim(Vec2(0, 0), Vec2(CentiMeter(30), CentiMeter(30))), entity->center_pos.xy))
        {
            
            Render_PushText(&game_state->render_context, Str8Lit("release your mouse to play the card"), Vec3(0, 0, CentiMeter(60)), Vec4(1,0,1,1), CoordinateType_World, FontKind_Arial);
            
            can_move_to_table = 1;
        }
        
        if (game_state->controller.left_mouse.released)
        {
            entity->is_pressed = false;
            entity->target_pos = entity->residency_pos;
            entity->entity_index_to_follow = 0;
            game_state->card_pressed_index = 0;
            entity->target_pos.z =  entity->residency_pos.z;
            
            if (can_move_to_table)
            {
                entity->target_y_angle = PI32;
                ChangeResidency(game_state, entity_index, Card_Residency_Table);
            }
        }
        
    }
    
    entity->y_angle = MoveTowards(entity->y_angle, entity->target_y_angle, entity->dy_angle * dt);
    
    // NOTE(fakhri): update dimension
    entity->current_dimension = Vec2MoveTowards(entity->current_dimension,
                                                entity->target_dimension,
                                                dt * entity->dDimension);
    
    MoveEntity(game_state, entity, 100, 10, 0.5f, dt);
}

internal void
UpdateCompanionEntity(Game_State *game_state, Entity *entity, f32 dt)
{
    Assert(entity->entity_index_to_follow);
    
    MoveEntity(game_state, entity, 85.f, 0.5f, 0.1f, dt);
    
#if 0    
    Entity *followed_entity = game_state->entities + entity->entity_index_to_follow;
    entity->center_pos.xy = ClampInsideRect(RectCentDim(followed_entity->center_pos.xy, followed_entity->current_dimension), entity->center_pos.xy);
#endif
    
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
    
    game_state->game_mode = Game_Mode_GAME;
    SetFlag(game_state->game_session.flags, SESSION_FLAG_HOST_FINISHED_SPLITTING_DECK);
    
#if TEST_ONE_CARD
    AddCardEntity(game_state, MakeCardType(Category_Hearts, Card_Number_Jack), Card_Residency_Down);
#else
    
    for (u32 player_index = 0;
         player_index < MAX_PLAYER_COUNT;
         ++player_index)
    {
        Player *player = game_state->game_session.players + player_index;
        player->joined = true;
        player->assigned_residency = (Card_Residency)(player_index + 1);
        player->username = PushStr8F(os->permanent_arena, "%s", "a");
    }
    
    for (u32 card_index = 0;
         card_index < 13;
         ++card_index)
    {
        AddCardEntity(game_state, MakeCardType(Category_Tiles, (Card_Number)card_index), Card_Residency_Up);
    }
    
    for (u32 card_index = 0;
         card_index < 1;
         ++card_index)
    {
        AddCardEntity(game_state, MakeCardType(Category_Tiles, (Card_Number)card_index), Card_Residency_Up);
    }
    
#if 1    
    for (u32 card_index = 1;
         card_index < 13;
         ++card_index)
    {
        AddCardEntity(game_state, MakeCardType(Category_Tiles, (Card_Number)card_index), Card_Residency_Down);
    }
#endif
    
    for (u32 card_index = 0;
         card_index < 1;
         ++card_index)
    {
        AddCardEntity(game_state, MakeCardType(Category_Clovers, (Card_Number)card_index), Card_Residency_Left);
    }
    
#if 1    
    for (u32 card_index = 1;
         card_index < 13;
         ++card_index)
    {
        AddCardEntity(game_state, MakeCardType(Category_Clovers, (Card_Number)card_index), Card_Residency_Down);
    }
#endif
    
    for (u32 card_index = 0;
         card_index < 1;
         ++card_index)
    {
        AddCardEntity(game_state, MakeCardType(Category_Pikes, (Card_Number)card_index), Card_Residency_Right);
    }
    
#if 1    
    for (u32 card_index = 1;
         card_index < 13;
         ++card_index)
    {
        AddCardEntity(game_state, MakeCardType(Category_Pikes, (Card_Number)card_index), Card_Residency_Down);
    }
#endif
    
    for (u32 card_index = 0;
         card_index < 13;
         ++card_index)
    {
        AddCardEntity(game_state, MakeCardType(Category_Hearts, (Card_Number)card_index), Card_Residency_Down);
    }
    
    Residency *burnt_residency = game_state->residencies + Card_Residency_Burnt;
#endif
}
