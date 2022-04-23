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

struct Residency
{
    u32 entity_indices[DECK_CARDS_COUNT];
    u32 entity_count;
    u32 controlling_player_id;
    
    b32 is_horizonal;
    b32 is_stacked;
    
    b32 needs_reorganizing;
    b32 burnable;
};

#endif //GAME_RESIDENCY_H
