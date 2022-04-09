
#define STB_TRUETYPE_IMPLEMENTATION
#include "third_party/stb_truetype.h"


internal void
LoadFont(Game_State *game_state, M_Arena *arena, Font_Kind font_kind)
{
    if (font_kind == FontKind_None) return;
    
    String8 path = {};
    Font *font = 0;
    f32 scale = 0;
    switch(font_kind)
    {
        case FontKind_Arial:
        {
            path = Str8Lit("data/fonts/arial.ttf");
            font = game_state->fonts + font_kind;
            scale = 50;
        } break;
        case FontKind_MenuTitle:
        {
            path = Str8Lit("data/fonts/arial.ttf");
            scale = 50;
            font = game_state->fonts + font_kind;
        } break;
        case FontKind_MenuItem:
        {
            path = Str8Lit("data/fonts/arial.ttf");
            font = game_state->fonts + font_kind;
            scale = 50;
        } break;
        default:
        {
            Assert(FontKind_None < font_kind && font_kind < FontKind_Count);
            NotImplemented;
        } break;
    }
    
    Assert(scale);
    Assert(font);
    
    M_Temp scratch = GetScratch(&arena, 1);
    
    // NOTE(fakhri): constants
    u32 direct_map_first = ' ';
    u32 direct_map_opl = 128;
    v2 oversample = { 1.f, 1.f };
    v2i32 atlas_size = Vec2i32(1024, 1024);
    
    String8 font_data = os->LoadEntireFile(scratch.arena, path);
    
    U8 *pixels = PushArrayZero(arena, U8, atlas_size.x * atlas_size.y);
    
    // NOTE(fakhri): calculate basic metrics
    F32 ascent = 0;
    F32 descent = 0;
    F32 line_gap = 0;
    stbtt_GetScaledFontVMetrics(font_data.str, 0, scale, &ascent, &descent, &line_gap);
    F32 line_advance = ascent - descent + line_gap;
    
    stbtt_pack_context ctx = {0};
    stbtt_PackBegin(&ctx, pixels, atlas_size.x, atlas_size.y, 0, 1, 0);
    stbtt_PackSetOversampling(&ctx, (u32)oversample.x, (u32)oversample.y);
    stbtt_packedchar *chardata_for_range = PushArrayZero(scratch.arena, stbtt_packedchar, direct_map_opl-direct_map_first);
    stbtt_pack_range rng =
    {
        scale,
        (int)direct_map_first,
        0,
        (int)(direct_map_opl - direct_map_first),
        chardata_for_range,
    };
    stbtt_PackFontRanges(&ctx, font_data.str, 0, &rng, 1);
    stbtt_PackEnd(&ctx);
    
    // NOTE(fakhri): build direct map
    v2 atlas_f32 = Vec2f32FromVec2i32(atlas_size);
    Glyph *direct_map = PushArrayZero(arena, Glyph, direct_map_opl-direct_map_first);
    for(U32 codepoint = direct_map_first; codepoint < direct_map_opl; codepoint += 1)
    {
        U32 index = codepoint - direct_map_first;
        F32 x_offset = 0;
        F32 y_offset = 0;
        stbtt_aligned_quad quad = {0};
        stbtt_GetPackedQuad(rng.chardata_for_range, atlas_size.width, atlas_size.height, index, &x_offset, &y_offset, &quad, 1);
        Glyph *glyph = direct_map + index;
        glyph->src = RectMinMax(Vec2(quad.s0, quad.t0),
                                Vec2(quad.s1, quad.t1));
        
        glyph->offset = Vec2(quad.x0, quad.y0);
        
        Rectangle2D src_px = RectMinMax(HadamardMultiplyVec2(Vec2(quad.s0, quad.t0), atlas_f32),
                                        HadamardMultiplyVec2(Vec2(quad.s1, quad.t1), atlas_f32));
        
        glyph->size = HadamardMultiplyVec2(RectDim(src_px),
                                           Vec2(1.f / oversample.x, 1.f / oversample.y));
        glyph->advance = x_offset;
    }
    
    
    font->map_first = direct_map_first;
    font->map_opl  = direct_map_opl;
    font->map = direct_map;
    font->texture = ReserveTexture2D(atlas_size, TextureFormat2D_R8);
    font->line_advance = line_advance;
    font->ascent = ascent;
    font->descent = descent;
    FillTexture2D(font->texture, Vec2i32(0, 0), atlas_size, Str8(pixels, atlas_size.x*atlas_size.y));
    
    ReleaseScratch(scratch);
}

internal void
ChangeActiveFont(Game_State *game_state, Font_Kind font_kind)
{
    Assert(FontKind_None < font_kind && font_kind < FontKind_Count);
    game_state->active_font = game_state->fonts + font_kind;
}


internal f32
GetFontWidth(Font *font, String8 text)
{
    f32 text_width = 0;
    for (u32 ch_index = 0;
         ch_index < text.size;
         ++ch_index)
    {
        u8 ch = text.str[ch_index];
        Assert(font->map_first <= ch && ch < font->map_opl);
        text_width += font->map[ch - font->map_first].advance;
    }
    
    return text_width;
}

internal f32
GetFontNormalizedWidth(Font *font, String8 text)
{
    f32 result = GetFontWidth(font, text);
    result *= 1.f / os->window_size.width;
    return result;
}

internal inline f32
GetFontHeight(Font *font)
{
    f32 result = font->ascent - font->descent;
    return result;
}

internal inline f32
GetFontNormalizedHeight(Font *font)
{
    f32 result = GetFontHeight(font);
    result *= 1.f / os->window_size.height;
    return result;
}
