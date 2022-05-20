/* date = March 12th 2022 0:28 pm */

#ifndef GAME_ENTITY_H
#define GAME_ENTITY_H

enum Entity_Type
{
    EntityType_Null_Entity,
    EntityType_Cursor_Entity, // NOTE(fakhri): always following mouse
    EntityType_Card,
    EntityType_Companion,
    EntityType_Button,
    EntityType_Numbers,
};

#define TEST_ONE_CARD 0


#define CARD_WIDTH  (MiliMeter(57.0f))
#define CARD_HEIGHT (MiliMeter(88.7f))

#define CARD_HORIZONTAL_GAP (-MiliMeter(10))
#define CARD_VIRTICAL_GAP (-CentiMeter(6))

#define CARD_HORIZONTAL_ADVANCE (CARD_WIDTH + CARD_HORIZONTAL_GAP)
#define CARD_VIRTICAL_ADVANCE (CARD_HEIGHT + CARD_VIRTICAL_GAP)
#define THRESHOLD_FOR_BURNING Category_Count

typedef u8 Card_Category;
enum
{
    Category_Hearts,  // red
    Category_Tiles,   // red
    Category_Clovers, // black
    Category_Pikes,   // black
    Category_Count,
};

#define InvalidCardNumber Card_Number_Count

typedef u8 Card_Number;
enum
{
    Card_Number_Ace,
    Card_Number_2,
    Card_Number_3,
    Card_Number_4,
    Card_Number_5,
    Card_Number_6,
    Card_Number_7,
    Card_Number_8,
    Card_Number_9,
    Card_Number_10,
    Card_Number_Jack,
    Card_Number_Queen,
    Card_Number_King,
    Card_Number_Count,
};

struct Card_Type
{
    Card_Category category;
    Card_Number   number;
};

internal Card_Type
MakeCardType(Card_Category category, Card_Number number)
{
    Card_Type result;
    result.category = category;
    result.number   = number;
    return result;
}

internal Compact_Card_Type
MakeCompactCardType(Card_Category category, Card_Number number)
{
    Compact_Card_Type result = (((Compact_Card_Type)category << 4) | (Compact_Card_Type)number);
    return result;
}

internal Card_Type
UnpackCompactCardType(Compact_Card_Type compact_card)
{
    Card_Category category = (Card_Category)(compact_card >> 4);
    Card_Number number = (Card_Number)(compact_card & 0xf);
    Card_Type result = MakeCardType(category, number);
    return result;
}

typedef u32 EntityFlags;
enum
{
    EntityFlag_UnderCursor       = 1 << 0,
    EntityFlag_Pressed           = 1 << 1,
    EntityFlag_DontDrawThisFrame = 1 << 2,
    EntityFlag_Selected          = 1 << 3,
    EntityFlag_MarkedForBurning  = 1 << 4,
};

typedef u32 ButtonEntityKind;
enum
{
    ButtonEntityKind_QuestionCredibility,
    ButtonEntityKind_PlaySelectedCards,
};

struct Entity
{
    Entity_Type type;
    v3 residency_pos;
    v3 center_pos;
    v3 target_pos;
    
    f32 y_angle;
    f32 target_y_angle;
    f32 dy_angle;
    
    v2 curr_dimension;
    v2 target_dimension;
    f32 dDimension;
    
    v2 velocity;
    
    // TODO(fakhri): we can have flags here instead, then each type of entity can have its own flags
    EntityFlags flags;
    
    EntityID entity_id_to_follow;
    v2  offset_in_follwed_entity;
    
    Card_Type card_type;
    
    ResidencyKind residency;
    Texture2D texture;
    
    ButtonEntityKind button_kind;
};

#endif //GAME_ENTITY_H
