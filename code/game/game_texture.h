/* date = March 29th 2022 10:33 pm */

#ifndef GAME_TEXTURE_H
#define GAME_TEXTURE_H

enum TextureFormat2D
{
    TextureFormat2D_Null,
    TextureFormat2D_R8,
    TextureFormat2D_RGB8,
    TextureFormat2D_RGBA8,
    TextureFormat2D_COUNT
};

struct Texture2D
{
    U32 id;
    TextureFormat2D format;
    v2i32 size;
};


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

typedef u32 TextureID;
enum
{
    TextureID_None,
    // NOTE(fakhri): categories
    TextureID_Clover,
    TextureID_Heart,
    TextureID_Pikes,
    TextureID_Tiles,
    // NOTE(fakhri): black numbers
    TextureID_BlackNumbers_Ace,
    TextureID_BlackNumbers_2,
    TextureID_BlackNumbers_3,
    TextureID_BlackNumbers_4,
    TextureID_BlackNumbers_5,
    TextureID_BlackNumbers_6,
    TextureID_BlackNumbers_7,
    TextureID_BlackNumbers_8,
    TextureID_BlackNumbers_9,
    TextureID_BlackNumbers_10,
    TextureID_BlackNumbers_Jack,
    TextureID_BlackNumbers_Queen,
    TextureID_BlackNumbers_King,
    // NOTE(fakhri): red numbers
    TextureID_RedNumbers_Ace,
    TextureID_RedNumbers_2,
    TextureID_RedNumbers_3,
    TextureID_RedNumbers_4,
    TextureID_RedNumbers_5,
    TextureID_RedNumbers_6,
    TextureID_RedNumbers_7,
    TextureID_RedNumbers_8,
    TextureID_RedNumbers_9,
    TextureID_RedNumbers_10,
    TextureID_RedNumbers_Jack,
    TextureID_RedNumbers_Queen,
    TextureID_RedNumbers_King,
    TextureID_Jack,
    TextureID_Queen = TextureID_Jack + Category_Count,
    TextureID_King  = TextureID_Queen + Category_Count,
    TextureID_Arrow = TextureID_King + Category_Count,
    TextureID_CardFrame,
    TextureID_CardBack,
    TextureID_Count,
};

#endif //GAME_TEXTURE_H
