
internal void
InitResidencies(Game_State *game_state)
{
    // NOTE(fakhri): up residency
    {
        Residency *residency = game_state->residencies + CardResidency_Up;
        SetFlag(residency->flags, ResidencyFlags_Horizontal | ResidencyFlags_Burnable);
        residency->controlling_player_id = InvalidePlayerID;
    }
    
    // NOTE(fakhri): down residency
    {
        Residency *residency = game_state->residencies + CardResidency_Down;
        SetFlag(residency->flags, ResidencyFlags_Horizontal | ResidencyFlags_Burnable);
        residency->controlling_player_id = InvalidePlayerID;
    }
    
    // NOTE(fakhri): left residency
    {
        Residency *residency = game_state->residencies + CardResidency_Left;
        SetFlag(residency->flags, ResidencyFlags_Burnable);
        residency->controlling_player_id = InvalidePlayerID;
    }
    
    // NOTE(fakhri): right residency
    {
        Residency *residency = game_state->residencies + CardResidency_Right;
        SetFlag(residency->flags, ResidencyFlags_Burnable);
        residency->controlling_player_id = InvalidePlayerID;
    }
    
    // NOTE(fakhri): table residency
    {
        Residency *residency = game_state->residencies + CardResidency_Table;
        SetFlag(residency->flags, ResidencyFlags_Stacked);
        residency->controlling_player_id = InvalidePlayerID;
    }
    
    // NOTE(fakhri): burnt residency
    {
        Residency *residency = game_state->residencies + CardResidency_Burnt;
        SetFlag(residency->flags, ResidencyFlags_Stacked);
        residency->controlling_player_id = InvalidePlayerID;
    }
    
    
    // NOTE(fakhri): card selecting residency
    {
        Residency *residency = game_state->residencies + CardResidency_CardSelecting;
        SetFlag(residency->flags, ResidencyFlags_Horizontal);
        residency->controlling_player_id = InvalidePlayerID;
    }
    
    // NOTE(fakhri): card declaring residency
    {
        Residency *residency = game_state->residencies + CardResidency_CardSelecting;
        SetFlag(residency->flags, ResidencyFlags_Horizontal);
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
        case CardResidency_Up:
        {
            result.x = -0.5f * (entity_count - 1) * (CARD_WIDTH + CARD_HORIZONTAL_GAP);
            result.y = start_point.y;
        } break;
        case CardResidency_Down:
        {
            result.x = -0.5f * (entity_count - 1) * (CARD_WIDTH + CARD_HORIZONTAL_GAP);
            result.y = -start_point.y;
        } break;
        case CardResidency_CardSelecting:
        {
            result.x = -0.5f * (entity_count - 1) * (CARD_WIDTH + CARD_HORIZONTAL_GAP);
            result.y = CentiMeter(10);
        } break;
        case CardResidency_CardDeclaring:
        {
            result.x = -0.5f * (entity_count - 1) * (CARD_WIDTH + CARD_HORIZONTAL_GAP);
            result.y = -CentiMeter(10);
        } break;
        case CardResidency_Left:
        {
            result.x = -start_point.x;
            result.y = 0.5f * (entity_count - 1) * (CARD_HEIGHT + CARD_VIRTICAL_GAP);
        } break;
        case CardResidency_Right:
        {
            result.x = +start_point.x;
            result.y = 0.5f * (entity_count - 1) * (CARD_HEIGHT + CARD_VIRTICAL_GAP);
        } break;
        case CardResidency_Table:
        {
            result = Vec2(-CentiMeter(5), 0);
        } break;
        
        case CardResidency_Burnt:
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
    if (residency_type != CardResidency_None)
    {
        Residency *residency = game_state->residencies + residency_type;
        u32 remaining_cards = residency->entity_count;
        
#define MAX_CARDS_PER_RESIDENCY_ROW 15
#define MAX_CARDS_PER_RESIDENCY_COLOMN 13
        u32 remaining_cards_per_row;
        if (HasFlag(residency->flags, ResidencyFlags_Horizontal))
        {
            remaining_cards_per_row = MAX_CARDS_PER_RESIDENCY_ROW;
        }
        else
        {
            remaining_cards_per_row = MAX_CARDS_PER_RESIDENCY_COLOMN;
        }
        
        
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
            if(HasFlag(residency->flags, ResidencyFlags_Horizontal))
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
                    
                    residency_pos.y = (residency_type == CardResidency_Up)? 
                        old_y - CARD_VIRTICAL_ADVANCE : old_y + CARD_VIRTICAL_ADVANCE;
                    
                    residency_pos.z -= CentiMeter(20);
                }
            }
            else
            {
                // NOTE(fakhri): veritcal residency
                if(HasFlag(residency->flags, ResidencyFlags_Stacked))
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
                        remaining_cards_per_row = MAX_CARDS_PER_RESIDENCY_COLOMN;
                        f32 old_x = residency_pos.x;
                        residency_pos.xy = FindFirstPositionInResidencyRow(game_state,
                                                                           residency_type, 
                                                                           Min(remaining_cards_per_row, remaining_cards));
                        
                        residency_pos.x = (residency_type == CardResidency_Right)? 
                            old_x - CARD_HORIZONTAL_ADVANCE : old_x + CARD_HORIZONTAL_ADVANCE;
                        
                    }
                }
                residency_pos.z += MiliMeter(20);
            }
        }
        ClearFlag(residency->flags, ResidencyFlags_NeedsReorganizing);
    }
}

internal void
AddToResidency(Game_State *game_state, u32 entity_index, Card_Residency residency)
{
    Entity *entity = game_state->entities + entity_index;
    Residency *entity_residency = game_state->residencies + residency;
    Assert(entity_residency->entity_count < ArrayCount(entity_residency->entity_indices));
    entity_residency->entity_indices[entity_residency->entity_count] = entity_index;
    ++entity_residency->entity_count;
    entity->residency = residency;
    SetFlag(entity_residency->flags, ResidencyFlags_NeedsReorganizing);
}

internal void
ChangeResidency(Game_State *game_state, u32 entity_index, Card_Residency residency)
{
    Entity *entity = game_state->entities + entity_index;
    Card_Residency old_residency = entity->residency;
    if (old_residency != residency)
    {
        // NOTE(fakhri): remove the entity from the old residency
        Residency *old_entity_residency = game_state->residencies + old_residency;
        for (u32 residency_index = 0;
             residency_index < old_entity_residency->entity_count;
             ++residency_index)
        {
            if (old_entity_residency->entity_indices[residency_index] == entity_index)
            {
                SetFlag(old_entity_residency->flags, ResidencyFlags_NeedsReorganizing);
                for (; residency_index + 1 < old_entity_residency->entity_count; ++residency_index)
                {
                    old_entity_residency->entity_indices[residency_index] = old_entity_residency->entity_indices[residency_index + 1];
                }
                --old_entity_residency->entity_count;
                break;
            }
        }
        // NOTE(fakhri): make sure that we found the entity in the old residency
        Assert(HasFlag(old_entity_residency->flags, ResidencyFlags_NeedsReorganizing));
        
        AddToResidency(game_state, entity_index, residency);
    }
    
}

internal void
MoveAllFromResidency(Game_State *game_state, Card_Residency from, Card_Residency to)
{
    Residency *from_residency = game_state->residencies + from;
    
    while(from_residency->entity_count)
    {
        u32 entity_index = from_residency->entity_indices[0];
        ChangeResidency(game_state, entity_index, to);
    }
    
}

internal void
AssignResidencyToPlayers(Game_State *game_state)
{
    u32 residency = CardResidency_Left;
    for(u32 player_index = 0;
        player_index < ArrayCount(game_state->players);
        ++player_index)
    {
        Player *player = game_state->players + player_index;
        if (player_index == game_state->my_player_id)
        {
            player->assigned_residency_index = CardResidency_Down;
            game_state->residencies[CardResidency_Down].controlling_player_id = player_index;
        }
        else
        {
            Assert(residency != CardResidency_Down);
            player->assigned_residency_index = (Card_Residency)residency;
            game_state->residencies[residency].controlling_player_id = player_index;
            ++residency;
        }
    }
}

internal b32
IsResidencyEmpty(Game_State *game_state, Card_Residency residency)
{
    b32 result = game_state->residencies[residency].entity_count == 0;
    return result;
}