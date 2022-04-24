/* date = April 23rd 2022 0:50 am */

#ifndef GAME_RESIDENCY_H
#define GAME_RESIDENCY_H

enum Card_Residency
{
    CardResidency_None,
    CardResidency_Left,
    CardResidency_Right,
    CardResidency_Up,
    CardResidency_Down,
    CardResidency_Table,
    CardResidency_Burnt,
    CardResidency_CardSelecting,
    CardResidency_CardDeclaring,
    
    CardResidency_Count,
};

enum Residency_Flags
{
    ResidencyFlags_Horizontal        = 1 << 0,
    ResidencyFlags_Stacked           = 1 << 1,
    ResidencyFlags_NeedsReorganizing = 1 << 2,
    ResidencyFlags_Burnable          = 1 << 3,
};

struct Residency
{
    u32 entity_indices[DECK_CARDS_COUNT];
    u32 entity_count;
    u32 controlling_player_id;
    
    i32 flags;
};

#endif //GAME_RESIDENCY_H
