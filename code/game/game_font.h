/* date = March 31st 2022 6:52 pm */

#ifndef GAME_FONTS_H
#define GAME_FONTS_H

enum Font_Kind
{
    FontKind_None,
    FontKind_Arial,
    FontKind_MenuTitle,
    FontKind_MenuItem,
    
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
};

#endif //GAME_FONTS_H