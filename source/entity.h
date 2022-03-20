/* date = March 12th 2022 0:28 pm */

#ifndef ENTITY_H
#define ENTITY_H

enum Entity_Type
{
    Entity_Type_Null_Entity,
    Entity_Type_Cursor_Entity, // NOTE(fakhri): always following mouse
    Entity_Type_Card,
    Entity_Type_Companion,
};

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

#define TEST_ONE_CARD 0

#if TEST_ONE_CARD
#define CARD_WIDTH  (2.f * 3.5f)
#define CARD_HEIGHT (2.f * 5.45f)
#else
#define CARD_WIDTH  (1.5f * 3.5f)
#define CARD_HEIGHT (1.5f * 5.45f)
#endif

#define CARD_X_GAP (-1.5f)
#define CARD_Y_GAP (-5.0f)


enum Card_Category
{
    Category_Hearts,  // red
    Category_Tiles,   // red
    Category_Clovers, // black
    Category_Pikes,   // black
    Category_Count,
};

enum Card_Number
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

struct Entity
{
    Entity_Type type;
    v3 residency_pos;
    v3 center_pos;
    v3 target_pos;
    
    f32 y_angle;
    f32 target_y_angle;
    f32 dy_angle;
    
    v2 current_dimension;
    v2 target_dimension;
    f32 dDimension;
    
    v2 velocity;
    
    // TODO(fakhri): we can have flags here instead, then each type of entity can have its own flags
    b32 is_under_cursor;
    b32 is_pressed;
    
    u32 followed_entity_index;
    v2  offset_in_follwed_entity;
    
    Card_Type card_type;
    
    Card_Residency residency;
    GLuint texture;
    
    b32 marked_for_burning;
};

#endif //ENTITY_H
