/* date = January 29th 2022 11:29 am */

#ifndef GAME_RENDER_H
#define GAME_RENDER_H

enum Render_Kind
{
    RenderKind_None,
    RenderKind_Quad,
    RenderKind_Image,
    RenderKind_Clear,
    
    RenderKind_Count,
};

enum Coordinate_Type
{
    CoordinateType_None,
    CoordinateType_Screen,
    CoordinateType_World,
    
    CoordinateType_Count,
};

struct RenderRequest_Header
{
    Render_Kind kind;
    f32 z;
    u32 offset_to_next;
};

struct RenderRequest_Quad
{
    RenderRequest_Header header;
    v2 screen_coords;
    v4 color;
    v2 size;
    f32 y_angle;
};

struct RenderRequest_Image
{
    RenderRequest_Header header;
    v2 screen_coords;
    Texture2D texture;
    b32 flip_y;
    v2 size;
    f32 y_angle;
    v4 color;
    v4 src;
};

struct RenderRequest_Clear
{
    RenderRequest_Header header;
    v4 color;
};

struct Push_Buffer
{
    void *memory;
    u32 capacity;
    u32 size;
    u32 requests_count;
};

struct Render_Context
{
    M_Arena *frame_arena;
    
    Shaders_Hash shaders_hash;
    Shader_Program shaders[ShaderKind_Count];
    Texture2D textures[TextureID_Count];
    
    Font fonts[FontKind_Count];
    Font_Kind active_font;
    
    Push_Buffer *push_buffer;
    
    f32 mouse_influence;
    v2 screen;
    v2 camera_position;
    f32 pixels_per_meter;
};

struct RenderSortEntry
{
    u32 offset;
    f32 z;
};

struct RenderSortBuffer
{
    RenderSortEntry *elements;
    u32 count;
};

#endif //GAME_RENDER_H
