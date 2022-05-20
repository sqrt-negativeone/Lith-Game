/* date = April 23rd 2022 0:50 am */

#ifndef GAME_RESIDENCY_H
#define GAME_RESIDENCY_H

#define EachValidResidencyKind(residency_kind)         \
/*for (*/u32 residency_kind = ResidencyKind_Nil + 1; \
residency_kind < ResidencyKind_Count;                \
++residency_kind/*)*/


#define EachControllableResidencyKind(residency_kind)  \
/*for (*/u32 residency_kind = ResidencyKind_Left;    \
residency_kind < ResidencyKind_Down;                 \
++residency_kind/*)*/

typedef u32 ResidencyKind;
enum
{
    ResidencyKind_Nil,
    ResidencyKind_Nonespacial,
    
    ResidencyKind_Left,
    ResidencyKind_Up,
    ResidencyKind_Right,
    ResidencyKind_Down,
    
    ResidencyKind_Table,
    ResidencyKind_Burnt,
    
    ResidencyKind_DeclarationOptions,
    ResidencyKind_SelectedCards,
    
    ResidencyKind_Count,
};

typedef u32 Residency_Flags;
enum 
{
    ResidencyFlags_Horizontal          = 1 << 0,
    ResidencyFlags_Stacked             = 1 << 1,
    ResidencyFlags_NeedsReorganizing   = 1 << 2,
    ResidencyFlags_Burnable            = 1 << 3,
    ResidencyFlags_Hidden              = 1 << 4,
    ResidencyFlags_OutsideScreen       = 1 << 5,
    ResidencyFlags_RandomizedPlacement = 1 << 6,
    ResidencyFlags_HostsCards          = 1 << 7,
};

struct Residency
{
    EntityID entity_ids[DECK_CARDS_COUNT];
    u32 entity_count;
    PlayerID controlling_player_id;
    Residency_Flags flags;
    v3 base_position;
    // +1 for overflowing to the same direction of the axis, -1 for the opposite direction
    f32 advance_direction;
};

struct ResidencyIterator
{
    ResidencyKind residency_kind;
    Residency *residency;
    i32 index; // index of the current entity in the residency
    b32 dont_increment;
};

#endif //GAME_RESIDENCY_H
