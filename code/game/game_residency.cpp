
internal ResidencyIterator
MakeResidencyIterator(Residency *residency)
{
    ResidencyIterator result;
    result.residency = residency;
    result.index = 0;
    return result;
}

#define IsValidEntityID(entity_id) ((entity_id) != InvalidEntityID)
#define EachValidResidencyEntityID(entity_id, iter) \
/*for(*/EntityID entity_id = ResidencyIterator_NextValid(&iter);  \
IsValidEntityID(entity_id);                                       \
entity_id = ResidencyIterator_NextValid(&iter) /*)*/

internal EntityID
ResidencyIterator_NextValid(ResidencyIterator *iter)
{
    EntityID result = InvalidEntityID;
    for (;iter->index < iter->residency->entity_count; ++iter->index)
    {
        EntityID entity_id = iter->residency->entity_ids[iter->index];
        if(IsValidEntityID(entity_id))
        {
            result = entity_id;
            ++iter->index;
            break;
        }
    }
    return result;
}

internal void
InitResidencies(Game_State *game_state)
{
    // NOTE(fakhri): none spacial residency
    {
        Residency *residency = game_state->residencies + ResidencyKind_Nonespacial;
        // TODO(fakhri): make use of these flags
        SetFlag(residency->flags, ResidencyFlags_OutsideScreen | ResidencyFlags_RandomizedPlacement);
        residency->controlling_player_id = InvalidePlayerID;
    }
    
    // NOTE(fakhri): up residency
    {
        Residency *residency = game_state->residencies + ResidencyKind_Up;
        SetFlag(residency->flags, ResidencyFlags_Horizontal | ResidencyFlags_Burnable | ResidencyFlags_HostsCards);
        residency->controlling_player_id = InvalidePlayerID;
        residency->base_position = Vec3(0, CentiMeter(20), 0);
        residency->advance_direction = -1.0f;
    }
    
    // NOTE(fakhri): down residency
    {
        Residency *residency = game_state->residencies + ResidencyKind_Down;
        SetFlag(residency->flags, ResidencyFlags_Horizontal | ResidencyFlags_Burnable | ResidencyFlags_HostsCards);
        residency->controlling_player_id = InvalidePlayerID;
        residency->base_position = Vec3(0, -CentiMeter(20), 0);
        residency->advance_direction = 1.0f;
    }
    
    // NOTE(fakhri): left residency
    {
        Residency *residency = game_state->residencies + ResidencyKind_Left;
        SetFlag(residency->flags, ResidencyFlags_Burnable | ResidencyFlags_HostsCards);
        residency->controlling_player_id = InvalidePlayerID;
        residency->base_position = Vec3(-CentiMeter(40), 0, 0);
        residency->advance_direction = 1.0f;
    }
    
    // NOTE(fakhri): right residency
    {
        Residency *residency = game_state->residencies + ResidencyKind_Right;
        SetFlag(residency->flags, ResidencyFlags_Burnable | ResidencyFlags_HostsCards);
        residency->controlling_player_id = InvalidePlayerID;
        residency->base_position = Vec3(CentiMeter(40), 0, 0);
        residency->advance_direction = -1.0f;
    }
    
    // NOTE(fakhri): table residency
    {
        Residency *residency = game_state->residencies + ResidencyKind_Table;
        SetFlag(residency->flags, ResidencyFlags_Stacked | ResidencyFlags_Hidden | ResidencyFlags_HostsCards);
        residency->controlling_player_id = InvalidePlayerID;
        residency->base_position = Vec3(-CentiMeter(5), 0, 0);
        residency->advance_direction = 1.0f;
    }
    
    // NOTE(fakhri): burnt residency
    {
        Residency *residency = game_state->residencies + ResidencyKind_Burnt;
        SetFlag(residency->flags, ResidencyFlags_Stacked | ResidencyFlags_HostsCards);
        residency->controlling_player_id = InvalidePlayerID;
        residency->base_position = Vec3(CentiMeter(5), 0, 0);
        residency->advance_direction = -1.0f;
    }
    
    // NOTE(fakhri): card declaring residency
    {
        Residency *residency = game_state->residencies + ResidencyKind_DeclarationOptions;
        SetFlag(residency->flags, ResidencyFlags_Horizontal);
        residency->controlling_player_id = InvalidePlayerID;
    }
    
    // NOTE(fakhri): selected cards for play residency
    {
        Residency *residency = game_state->residencies + ResidencyKind_SelectedCards;
        SetFlag(residency->flags, ResidencyFlags_Horizontal | ResidencyFlags_HostsCards);
        residency->controlling_player_id = InvalidePlayerID;
        residency->base_position = Vec3(0, CentiMeter(10), 0);
    }
    
}


internal void
ReorganizeResidencyCards(Game_State *game_state, ResidencyKind residency_kind)
{
    if (residency_kind != ResidencyKind_Nil)
    {
        Residency *residency = game_state->residencies + residency_kind;
        
        // NOTE(fakhri): remove gaps in residency
        {
            u32 min_invalide_index = 0;
            for (u32 index = 0;
                 index < residency->entity_count;
                 ++index)
            {
                EntityID entity_id = residency->entity_ids[index];
                if (entity_id != InvalidEntityID)
                {
                    residency->entity_ids[min_invalide_index] = entity_id;
                    min_invalide_index++;
                }
            }
            residency->entity_count = min_invalide_index;
        }
        
        if (residency->entity_count)
        {
            // NOTE(fakhri): reorganize residency
            u32 reorganized_entity_count = 0;
            u32 axis_count_limit = 0;
            v2 advance = {};
            Axis2 axis = Axis2_X;
            Axis2 overflow_axis = Axis2_X;
            Rand_Ctx rand_ctx = {};
            v3 base_position = residency->base_position;
            axis_count_limit = residency->entity_count;
            if (!HasFlag(residency->flags, ResidencyFlags_RandomizedPlacement))
            {
                if (HasFlag(residency->flags, ResidencyFlags_Horizontal))
                {
                    axis = Axis2_X;
                    overflow_axis = Axis2_Y;
                    axis_count_limit = 15;
                    advance = Vec2(CARD_HORIZONTAL_ADVANCE, CARD_VIRTICAL_ADVANCE);
                }
                else if (HasFlag(residency->flags, ResidencyFlags_Stacked))
                {
                    axis = Axis2_Y;
                    advance = Vec2(0, MiliMeter(0.24f));
                }
                else
                {
                    axis = Axis2_Y;
                    overflow_axis = Axis2_X;
                    axis_count_limit = 13;
                    advance = Vec2(CARD_HORIZONTAL_ADVANCE, CARD_VIRTICAL_ADVANCE);
                }
                
                base_position[axis] = -0.5f * Min(axis_count_limit, residency->entity_count) * advance[axis];
                advance[overflow_axis] *= residency->advance_direction;
            }
            else
            {
                u32 seed = *(u32*)&os->time.wall_time;
                rand_ctx = MakeLineraRandomGenerator(seed);
            }
            
            Assert(axis_count_limit);
            u32 total_row_count = residency->entity_count / axis_count_limit;
            u32 remaining_entity_count = residency->entity_count % axis_count_limit;
            u32 current_row = 0;
            
            ResidencyIterator iter = MakeResidencyIterator(residency);
            for(EachValidResidencyEntityID(entity_id, iter))
            {
                v3 position = {};
                
                if (HasFlag(residency->flags, ResidencyFlags_RandomizedPlacement))
                {
                    f32 angle = 2.0f * PI32 * NextRandomNumberNF(&rand_ctx);
                    f32 length;
                    v3 dir = Vec3(CosF(angle), SinF(angle), 0);
                    if (HasFlag(residency->flags, ResidencyFlags_OutsideScreen))
                    {
                        Render_Context *render_context = &game_state->render_context;
                        length = LengthVec2(Vec2(CARD_WIDTH, CARD_HEIGHT)) + (1.0f + render_context->mouse_influence) * LengthVec2(render_context->screen) / render_context->pixels_per_meter;
                    }
                    else
                    {
                        length = CentiMeter(10) * NextRandomNumberNF(&rand_ctx);
                    }
                    
                    position = length * dir;
                }
                else
                {
                    ++reorganized_entity_count;
                    if (reorganized_entity_count > axis_count_limit)
                    {
                        // NOTE(fakhri): handle breaking the row
                        ++current_row;
                        reorganized_entity_count -= axis_count_limit;
                        base_position[overflow_axis] += advance[overflow_axis];
                        if (current_row == total_row_count)
                        {
                            base_position[axis] = -0.5f * remaining_entity_count * advance[axis];
                        }
                        base_position.z -= CentiMeter(20);
                    }
                    position = base_position;
                    position[axis] += reorganized_entity_count * advance[axis];
                    position.z += MiliMeter(20);
                }
                
                Entity *entity = game_state->entities + entity_id;
                entity->residency_pos = position;
                if (entity->entity_id_to_follow == InvalidEntityID)
                {
                    // NOTE(fakhri): go to the residency position if i'm not already following an entitiy
                    entity->target_pos = entity->residency_pos;
                }
            }
            ClearFlag(residency->flags, ResidencyFlags_NeedsReorganizing);
        }
    }
}

internal void
AddToResidency(Game_State *game_state, u32 entity_id, ResidencyKind residency)
{
    Entity *entity = game_state->entities + entity_id;
    Residency *entity_residency = game_state->residencies + residency;
    // NOTE(fakhri): find an empty position where to place the entity
    u32 entity_place = 0;
    for (;
         entity_place < ArrayCount(entity_residency->entity_ids);
         ++entity_place)
    {
        EntityID entity_id = entity_residency->entity_ids[entity_place];
        if (entity_id == InvalidEntityID)
        {
            break;
        }
    }
    Assert(entity_place < ArrayCount(entity_residency->entity_ids));
    
    entity_residency->entity_ids[entity_place] = entity_id;
    ++entity_residency->entity_count;
    entity->residency = residency;
    SetFlag(entity_residency->flags, ResidencyFlags_NeedsReorganizing);
    
    if (HasFlag(entity_residency->flags, ResidencyFlags_Hidden))
    {
        entity->target_y_angle = PI32;
    }
    else
    {
        entity->target_y_angle = 0.0f;
    }
    
}

internal void
ChangeResidency(Game_State *game_state, u32 entity_id, ResidencyKind residency)
{
    Entity *entity = game_state->entities + entity_id;
    ResidencyKind old_residency = entity->residency;
    if (old_residency != residency)
    {
        // NOTE(fakhri): remove the entity from the old residency
        if (old_residency != ResidencyKind_Nil)
        {
            Residency *old_entity_residency = game_state->residencies + old_residency;
            for (u32 residency_index = 0;
                 residency_index < old_entity_residency->entity_count;
                 ++residency_index)
            {
                if (old_entity_residency->entity_ids[residency_index] == entity_id)
                {
                    SetFlag(old_entity_residency->flags, ResidencyFlags_NeedsReorganizing);
                    old_entity_residency->entity_ids[residency_index] = InvalidEntityID;
                    break;
                }
            }
            // NOTE(fakhri): make sure that we found the entity in the old residency
            Assert(HasFlag(old_entity_residency->flags, ResidencyFlags_NeedsReorganizing));
        }
        AddToResidency(game_state, entity_id, residency);
    }
}

internal void
MoveAllFromResidency(Game_State *game_state, ResidencyKind from, ResidencyKind to)
{
    ResidencyIterator iter = MakeResidencyIterator(game_state->residencies + from);
    for(EachValidResidencyEntityID(entity_id, iter))
    {
        ChangeResidency(game_state, entity_id, to);
    }
}

internal void
AssignResidencyToPlayers(Game_State *game_state)
{
    u32 residency = ResidencyKind_Left;
    for(u32 player_index = 0;
        player_index < ArrayCount(game_state->players);
        ++player_index)
    {
        Player *player = game_state->players + player_index;
        if (player_index == game_state->my_player_id)
        {
            player->assigned_residency_index = ResidencyKind_Down;
            game_state->residencies[ResidencyKind_Down].controlling_player_id = player_index;
        }
        else
        {
            Assert(residency != ResidencyKind_Down);
            player->assigned_residency_index = (ResidencyKind)residency;
            game_state->residencies[residency].controlling_player_id = player_index;
            ++residency;
        }
    }
}

internal b32
IsResidencyEmpty(Game_State *game_state, ResidencyKind residency)
{
    b32 result = game_state->residencies[residency].entity_count == 0;
    return result;
}

internal ResidencyKind
GetResidencyOfCurrentPlayer(Game_State *game_state)
{
    u32 curr_player_id = game_state->curr_player_id;
    ResidencyKind result = (ResidencyKind)game_state->players[curr_player_id].assigned_residency_index;
    return result;
}
