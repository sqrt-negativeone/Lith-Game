
internal ResidencyIterator
MakeResidencyIterator(Game_State *game_state, ResidencyKind residency_kind)
{
    ResidencyIterator result;
    result.residency_kind = residency_kind;
    result.residency = game_state->residencies + residency_kind;
    result.index = -1;
    result.dont_increment = 0;
    return result;
}

#define IsValidEntityID(entity_id) ((entity_id) != InvalidEntityID)
#define EachValidResidencyEntityID(entity_id, iter) \
/*for(*/EntityID entity_id = ResidencyIterator_Next(&iter);  \
IsValidEntityID(entity_id);                                       \
entity_id = ResidencyIterator_Next(&iter) /*)*/

internal EntityID
ResidencyIterator_Next(ResidencyIterator *iter)
{
    EntityID result = InvalidEntityID;
    
    iter->index = (iter->dont_increment)? iter->index : iter->index + 1;
    iter->dont_increment = 0;
    
    if (0 <= iter->index && (u32)iter->index < iter->residency->entity_count)
    {
        result = iter->residency->entity_ids[iter->index];
    }
    
    return result;
}

internal void
InitResidencies(Game_State *game_state)
{
    // NOTE(fakhri): none spacial residency
    {
        Residency *residency = game_state->residencies + ResidencyKind_Nonespacial;
        residency->flags = 0;
        SetFlag(residency->flags, ResidencyFlags_OutsideScreen | ResidencyFlags_RandomizedPlacement);
        residency->controlling_player_id = InvalidePlayerID;
        residency->entity_count = 0;
    }
    
    // NOTE(fakhri): left residency
    {
        Residency *residency = game_state->residencies + ResidencyKind_Left;
        residency->flags = 0;
        SetFlag(residency->flags, ResidencyFlags_Burnable | ResidencyFlags_HostsCards | ResidencyFlags_Hidden);
        residency->controlling_player_id = InvalidePlayerID;
        residency->base_position = Vec3(-CentiMeter(40), 0, 0);
        residency->advance_direction = 1.0f;
        residency->entity_count = 0;
    }
    
    // NOTE(fakhri): up residency
    {
        Residency *residency = game_state->residencies + ResidencyKind_Up;
        residency->flags = 0;
        SetFlag(residency->flags, ResidencyFlags_Horizontal | ResidencyFlags_Burnable | ResidencyFlags_HostsCards | ResidencyFlags_Hidden);
        residency->controlling_player_id = InvalidePlayerID;
        residency->base_position = Vec3(0, CentiMeter(20), 0);
        residency->advance_direction = -1.0f;
        residency->entity_count = 0;
    }
    
    // NOTE(fakhri): right residency
    {
        Residency *residency = game_state->residencies + ResidencyKind_Right;
        residency->flags = 0;
        SetFlag(residency->flags, ResidencyFlags_Burnable | ResidencyFlags_HostsCards | ResidencyFlags_Hidden);
        residency->controlling_player_id = InvalidePlayerID;
        residency->base_position = Vec3(CentiMeter(40), 0, 0);
        residency->advance_direction = -1.0f;
        residency->entity_count = 0;
    }
    
    // NOTE(fakhri): down residency
    {
        Residency *residency = game_state->residencies + ResidencyKind_Down;
        residency->flags = 0;
        SetFlag(residency->flags, ResidencyFlags_Horizontal | ResidencyFlags_Burnable | ResidencyFlags_HostsCards);
        residency->controlling_player_id = InvalidePlayerID;
        residency->base_position = Vec3(0, -CentiMeter(20), 0);
        residency->advance_direction = 1.0f;
        residency->entity_count = 0;
    }
    
    // NOTE(fakhri): table residency
    {
        Residency *residency = game_state->residencies + ResidencyKind_Table;
        residency->flags = 0;
        SetFlag(residency->flags, ResidencyFlags_Stacked | ResidencyFlags_Hidden | ResidencyFlags_HostsCards);
        residency->controlling_player_id = InvalidePlayerID;
        residency->base_position = Vec3(-CentiMeter(5), 0, 0);
        residency->advance_direction = 1.0f;
        residency->entity_count = 0;
    }
    
    // NOTE(fakhri): burnt residency
    {
        Residency *residency = game_state->residencies + ResidencyKind_Burnt;
        residency->flags = 0;
        SetFlag(residency->flags, ResidencyFlags_Stacked | ResidencyFlags_HostsCards);
        residency->controlling_player_id = InvalidePlayerID;
        residency->base_position = Vec3(CentiMeter(5), 0, 0);
        residency->advance_direction = -1.0f;
        residency->entity_count = 0;
    }
    
    // NOTE(fakhri): card declaring residency
    {
        Residency *residency = game_state->residencies + ResidencyKind_DeclarationOptions;
        residency->flags = 0;
        SetFlag(residency->flags, ResidencyFlags_Horizontal);
        residency->controlling_player_id = InvalidePlayerID;
        residency->base_position = Vec3(0, CentiMeter(0), Meter(1));
        residency->entity_count = 0;
    }
    
    // NOTE(fakhri): selected cards for play residency
    {
        Residency *residency = game_state->residencies + ResidencyKind_SelectedCards;
        residency->flags = 0;
        SetFlag(residency->flags, ResidencyFlags_Horizontal | ResidencyFlags_HostsCards);
        residency->controlling_player_id = InvalidePlayerID;
        residency->base_position = Vec3(0, CentiMeter(10), Meter(1));
        residency->entity_count = 0;
    }
    
}

internal void
ReorganizeResidencyCards(Game_State *game_state, ResidencyKind residency_kind)
{
    if (residency_kind != ResidencyKind_Nil)
    {
        Residency *residency = game_state->residencies + residency_kind;
        if (residency->entity_count)
        {
            // NOTE(fakhri): reorganize residency
            u32 reorganized_entity_count = 0;
            u32 axis_count_limit = 0;
            v2 advance = {};
            Axis2 axis = Axis2_X;
            Axis2 overflow_axis = Axis2_X;
            Rand_Ctx rand_ctx = {};
            v2 base_position = residency->base_position.xy;
            f32 z = residency->base_position.z;
            axis_count_limit = residency->entity_count;
            
            if (HasFlag(residency->flags, ResidencyFlags_RandomizedPlacement))
            {
                u32 seed = *(u32*)&os->time.wall_time;
                rand_ctx = MakeLineraRandomGenerator(seed);
            }
            else
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
            
            u32 total_row_count = residency->entity_count / axis_count_limit;
            u32 remaining_entity_count = residency->entity_count % axis_count_limit;
            u32 current_row = 0;
            
            ResidencyIterator iter = MakeResidencyIterator(game_state, residency_kind);
            for(EachValidResidencyEntityID(entity_id, iter))
            {
                v2 position = {};
                
                if (HasFlag(residency->flags, ResidencyFlags_RandomizedPlacement))
                {
                    f32 angle = 2.0f * PI32 * NextRandomNumberNF(&rand_ctx);
                    f32 length;
                    v2 dir = Vec2(CosF(angle), SinF(angle));
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
                        z -= CentiMeter(20);
                    }
                    position = base_position;
                    position[axis] += reorganized_entity_count * advance[axis];
                    z += MiliMeter(1);
                }
                
                Entity *entity = game_state->entities + entity_id;
                entity->residency_pos.xy = position;
                entity->residency_pos.z = z;
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
AddToResidency(Game_State *game_state, u32 entity_id, ResidencyKind residency_kind)
{
    Entity *entity = game_state->entities + entity_id;
    Residency *residency = game_state->residencies + residency_kind;
    
    u32 entity_place = residency->entity_count;
    Assert(entity_place < ArrayCount(residency->entity_ids));
    
    residency->entity_ids[entity_place] = entity_id;
    ++residency->entity_count;
    entity->residency = residency_kind;
    SetFlag(residency->flags, ResidencyFlags_NeedsReorganizing);
    
    if (HasFlag(residency->flags, ResidencyFlags_Hidden))
    {
        entity->target_orientation.y = PI32;
        
    }
    else
    {
        entity->target_orientation.y = 0.0f;
    }
    
}

internal void
ChangeResidency(Game_State *game_state, ResidencyIterator *res_iter, ResidencyKind residency_kind)
{
    if (res_iter->residency_kind != residency_kind)
    {
        EntityID entity_id = InvalidEntityID; 
        // NOTE(fakhri): remove the entity from the old residency
        if (res_iter->residency_kind != ResidencyKind_Nil)
        {
            SetFlag(res_iter->residency->flags, ResidencyFlags_NeedsReorganizing);
            Residency *residency = res_iter->residency;
            entity_id = residency->entity_ids[res_iter->index];
            --residency->entity_count;
            for (u32 index = res_iter->index;
                 index < residency->entity_count;
                 ++index)
            {
                residency->entity_ids[index] = residency->entity_ids[index + 1];
            }
            res_iter->dont_increment = 1;
        }
        AddToResidency(game_state, entity_id, residency_kind);
    }
}

internal void
ChangeResidency(Game_State *game_state, EntityID entity_id, ResidencyKind residency_kind)
{
    Entity *entity = game_state->entities + entity_id;
    ResidencyKind old_residency_kind = entity->residency;;
    Residency *old_residency = game_state->residencies + old_residency_kind;
    if (old_residency_kind != residency_kind)
    {
        // NOTE(fakhri): remove the entity from the old residency
        if (old_residency_kind != ResidencyKind_Nil)
        {
            SetFlag(old_residency->flags, ResidencyFlags_NeedsReorganizing);
            u32 index_in_res = 0;
            for(;
                index_in_res < old_residency->entity_count;
                ++index_in_res)
            {
                if (old_residency->entity_ids[index_in_res] == entity_id)
                {
                    break;
                }
            }
            Assert(index_in_res < old_residency->entity_count);
            --old_residency->entity_count;
            for (;
                 index_in_res < old_residency->entity_count;
                 ++index_in_res)
            {
                old_residency->entity_ids[index_in_res] = old_residency->entity_ids[index_in_res + 1];
            }
        }
        AddToResidency(game_state, entity_id, residency_kind);
    }
}

internal void
MoveAllFromResidency(Game_State *game_state, ResidencyKind from, ResidencyKind to)
{
    ResidencyIterator iter = MakeResidencyIterator(game_state, from);
    for(EachValidResidencyEntityID(entity_id, iter))
    {
        ChangeResidency(game_state, &iter, to);
    }
}

internal void
AssignResidencyToPlayers(Game_State *game_state)
{
    u32 residency = ResidencyKind_Left;
    
    PlayerID player_id = game_state->my_player_id;
    game_state->players[player_id].assigned_residency_kind = ResidencyKind_Down;
    game_state->residencies[ResidencyKind_Down].controlling_player_id = player_id;
    player_id += 1;
    player_id %= ArrayCount(game_state->players);
    
    game_state->players[player_id].assigned_residency_kind = ResidencyKind_Left;
    game_state->residencies[ResidencyKind_Left].controlling_player_id = player_id;
    player_id += 1;
    player_id %= ArrayCount(game_state->players);
    
    game_state->players[player_id].assigned_residency_kind = ResidencyKind_Up;
    game_state->residencies[ResidencyKind_Up].controlling_player_id = player_id;
    player_id += 1;
    player_id %= ArrayCount(game_state->players);
    
    game_state->players[player_id].assigned_residency_kind = ResidencyKind_Right;
    game_state->residencies[ResidencyKind_Right].controlling_player_id = player_id;
    
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
    ResidencyKind result = (ResidencyKind)game_state->players[curr_player_id].assigned_residency_kind;
    return result;
}
