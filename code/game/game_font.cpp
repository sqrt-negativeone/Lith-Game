
#define STB_TRUETYPE_IMPLEMENTATION
#include "third_party/stb_truetype.h"

struct FontInfo
{
    String8 path;
    f32 scale;
};

internal void
LoadFont(Render_Context *render_context, M_Arena *arena, Font_Kind font_kind)
{
    if (font_kind == FontKind_None) return;
    Assert(font_kind < FontKind_Count);
    Font *font = render_context->fonts + font_kind;
    MemoryZeroStruct(font);
    FontInfo font_info = {};
    switch(font_kind)
    {
        case FontKind_Arial:
        {
            font_info.path = Str8LitComp("data/fonts/arial.ttf");
            font_info.scale = 50.f;
        } break;
        case FontKind_GameTitle:
        {
            font_info.path = Str8LitComp("data/fonts/PilotCommand.otf");
            font_info.scale = 100.f;
        } break;
        case FontKind_MenuTitle:
        {
            font_info.path = Str8LitComp("data/fonts/PilotCommand.otf");
            font_info.scale = 60.f;
        } break;
        case FontKind_MenuItem:
        {
            font_info.path = Str8LitComp("data/fonts/BubbleboddyNeue.ttf");
            font_info.scale = 50.f;
        } break;
        case FontKind_InputField:
        {
            font_info.path = Str8LitComp("data/fonts/ariblk.ttf");
            font_info.scale = 50.f;
        } break;
        case FontKind_GameButton:
        {
            font_info.path = Str8LitComp("data/fonts/BubbleboddyNeue.ttf");
            font_info.scale = 30.f;
        } break;
        default: NotImplemented; break;
    }
    
    M_Temp scratch = GetScratch(&arena, 1);
    
    // NOTE(fakhri): constants
    u32 direct_map_first = ' ';
    u32 direct_map_opl = 128;
    v2 oversample = { 1.f, 1.f };
    v2i32 atlas_size = Vec2i32(1024, 1024);
    
    String8 font_data = os->LoadEntireFile(scratch.arena, font_info.path);
    
    U8 *pixels = PushArrayZero(arena, U8, atlas_size.x * atlas_size.y);
    Assert(pixels && font_data.str);
    
    // NOTE(fakhri): calculate basic metrics
    F32 ascent = 0;
    F32 descent = 0;
    F32 line_gap = 0;
    stbtt_GetScaledFontVMetrics(font_data.str, 0, font_info.scale, &ascent, &descent, &line_gap);
    F32 line_advance = ascent - descent + line_gap;
    
    stbtt_pack_context ctx = {0};
    stbtt_PackBegin(&ctx, pixels, atlas_size.x, atlas_size.y, 0, 1, 0);
    stbtt_PackSetOversampling(&ctx, (u32)oversample.x, (u32)oversample.y);
    stbtt_packedchar *chardata_for_range = PushArrayZero(scratch.arena, stbtt_packedchar, direct_map_opl-direct_map_first);
    Assert(chardata_for_range);
    stbtt_pack_range rng =
    {
        font_info.scale,
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
    Log("map size: %d", direct_map_opl-direct_map_first);
    Assert(direct_map);
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
        if (font->max_advance < glyph->advance)
        {
            font->max_advance = glyph->advance;
        }
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

internal Font *
GetFontOrActiveIfNone(Render_Context *render_context, Font_Kind font_kind)
{
    if (font_kind == FontKind_None)
    {
        font_kind = render_context->active_font;
    }
    Assert(font_kind > FontKind_None && font_kind < FontKind_Count);
    Font *font = render_context->fonts + font_kind;
    return font;
}

internal f32
GetFontMaxWidth(Render_Context *render_context, Font_Kind font_kind, u32 characters_count)
{
    f32 text_width = 0;
    
    Font *font = GetFontOrActiveIfNone(render_context, font_kind);
    text_width = font->max_advance * characters_count;
    return text_width;
}

internal f32
GetFontWidth(Render_Context *render_context, Font_Kind font_kind, String8 text)
{
    f32 text_width = 0;
    
    Font *font = GetFontOrActiveIfNone(render_context, font_kind);
    
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

internal inline f32
GetFontHeight(Render_Context *render_context, Font_Kind font_kind)
{
    Font *font = GetFontOrActiveIfNone(render_context, font_kind);
    f32 result = font->ascent - font->descent;
    return result;
}

internal inline void
ChangeActiveFont(Render_Context *render_context, Font_Kind font_kind)
{
    Assert(font_kind < FontKind_Count);
    render_context->active_font = font_kind;
}

internal f32
GetCharacterAdvance(Render_Context *render_context, Font_Kind font_kind, u8 character)
{
    Font *font = GetFontOrActiveIfNone(render_context, font_kind);
    f32 result = font->map[character - font->map_first].advance;
    return result;
}