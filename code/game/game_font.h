/* date = March 31st 2022 6:52 pm */

#ifndef GAME_FONTS_H
#define GAME_FONTS_H

typedef u32 Font_Kind;
enum
{
    FontKind_None,
    FontKind_Arial,
    FontKind_GameTitle,
    FontKind_MenuTitle,
    FontKind_MenuItem,
    FontKind_InputField,
    
    FontKind_Count,
};


struct Glyph
{
    Rectangle2D src;
    v2 offset;
    v2 size;
    F32 advance;
};


struct Font
{
    u32 map_first;
    u32 map_opl;
    Glyph *map;
    
    F32 line_advance;
    F32 ascent;
    F32 descent;
    Texture2D texture;
    f32 max_advance;
};

#endif //GAME_FONTS_H
