/* date = April 23rd 2022 0:50 am */

#ifndef GAME_RESIDENCY_H
#define GAME_RESIDENCY_H

enum Card_Residency
{
    Card_Residency_None,
    Card_Residency_Left,
    Card_Residency_Right,
    Card_Residency_Up,
    Card_Residency_Down,
    Card_Residency_Table,
    Card_Residency_Burnt,
    
    Card_Residency_Count,
};

enum Residency_Flags
{
    ResidencyFlags_Horizontal        = 1 << 0,
    ResidencyFlags_Stacked           = 1 << 1,
    ResidencyFlags_NeedsReorganizing = 1 << 2,
    ResidencyFlags_Burnable          = 1 << 3,
    ResidencyFlags_WrapDown          = 1 << 4,
    ResidencyFlags_WrapLeft          = 1 << 5,
};

struct Residency
{
    u32 entity_indices[DECK_CARDS_COUNT];
    u32 entity_count;
    u32 controlling_player_id;
    
    i32 flags;
};

#endif //GAME_RESIDENCY_H
