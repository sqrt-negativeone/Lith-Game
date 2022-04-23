
internal void
InitResidencies(Game_State *game_state)
{
    // NOTE(fakhri): up residency
    {
        Residency *residency = game_state->residencies + Card_Residency_Up;
        residency->is_horizonal = true;
        residency->is_stacked   = false;
        residency->burnable     = true;
        residency->controlling_player_id = InvalidePlayerID;
    }
    
    // NOTE(fakhri): down residency
    {
        Residency *residency = game_state->residencies + Card_Residency_Down;
        residency->is_horizonal = true;
        residency->is_stacked   = false;
        residency->burnable     = true;
        residency->controlling_player_id = InvalidePlayerID;
    }
    
    // NOTE(fakhri): left residency
    {
        Residency *residency = game_state->residencies + Card_Residency_Left;
        residency->is_horizonal = false;
        residency->is_stacked   = false;
        residency->burnable     = true;
        residency->controlling_player_id = InvalidePlayerID;
    }
    
    // NOTE(fakhri): right residency
    {
        Residency *residency = game_state->residencies + Card_Residency_Right;
        residency->is_horizonal = false;
        residency->is_stacked   = false;
        residency->burnable     = true;
        residency->controlling_player_id = InvalidePlayerID;
    }
    
    // NOTE(fakhri): table residency
    {
        Residency *residency = game_state->residencies + Card_Residency_Table;
        residency->is_horizonal = false;
        residency->is_stacked   = true;
        residency->burnable     = false;
        residency->controlling_player_id = InvalidePlayerID;
    }
    
    // NOTE(fakhri): burnt residency
    {
        Residency *residency = game_state->residencies + Card_Residency_Burnt;
        residency->is_horizonal = false;
        residency->is_stacked   = true;
        residency->burnable     = false;
        residency->controlling_player_id = InvalidePlayerID;
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
            result.x = -0.5f * (entity_count - 1) * (CARD_WIDTH + CARD_HORIZONTAL_GAP);
            result.y = start_point.y;
        } break;
        case Card_Residency_Down:
        {
            result.x = -0.5f * (entity_count - 1) * (CARD_WIDTH + CARD_HORIZONTAL_GAP);
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
        residency->needs_reorganizing = false;
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
            Residency *old_entity_residency = game_state->residencies + old_residency;
            for (u32 residency_index = 0;
                 residency_index < old_entity_residency->entity_count;
                 ++residency_index)
            {
                if (old_entity_residency->entity_indices[residency_index] == entity_index)
                {
                    old_entity_residency->needs_reorganizing = true;
                    for (; residency_index + 1 < old_entity_residency->entity_count; ++residency_index)
                    {
                        old_entity_residency->entity_indices[residency_index] = old_entity_residency->entity_indices[residency_index + 1];
                    }
                    --old_entity_residency->entity_count;
                    break;
                }
            }
            // NOTE(fakhri): make sure that we found the entity in the old residency
            Assert(old_entity_residency->needs_reorganizing);
        }
        
        // NOTE(fakhri): add entity to the new residency
        Residency *entity_residency = game_state->residencies + residency;
        Assert(entity_residency->entity_count < ArrayCount(entity_residency->entity_indices));
        entity_residency->entity_indices[entity_residency->entity_count] = entity_index;
        ++entity_residency->entity_count;
        entity->residency = residency;
        entity_residency->needs_reorganizing = true;
    }
}
