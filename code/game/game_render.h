/* date = January 29th 2022 11:29 am */

#ifndef GAME_RENDER_H
#define GAME_RENDER_H

struct Frensh_Suited_Cards_Texture
{
    Texture2D card_frame_texture;
    Texture2D card_back_texture;
    
    Texture2D clovers_up;
    Texture2D hearts_up;
    Texture2D pikes_up;
    Texture2D clovers_down;
    Texture2D hearts_down;
    Texture2D pikes_down;
    Texture2D tiles;
    
    Texture2D black_numbers_up[13];
    Texture2D black_numbers_down[13];
    Texture2D red_numbers_up[13];
    Texture2D red_numbers_down[13];
    
    Texture2D jacks[4];
    Texture2D queens[4];
    Texture2D kings[4];
};

enum Render_Kind
{
    RenderKind_None,
    RenderKind_Quad,
    RenderKind_Image,
    RenderKind_Text,
    
    RenderKind_Count,
};

enum Coordinate_Type
{
    CoordinateType_None,
    CoordinateType_Screen,
    CoordinateType_World,
    
    CoordinateType_Count,
};

struct Render_Quad_Request
{
    v4 color;
    v2 size;
    f32 y_angle;
};

struct Render_Text_Request
{
    String text;
    Font_Kind font_to_use;
    v4 color;
};

struct Render_Image_Request
{
    Texture2D texture;
    v2 size;
    f32 y_angle;
};

struct Render_Request
{
    Render_Kind kind;
    v3 screen_coords;
    
    union
    {
        Render_Quad_Request quad_request;
        Render_Text_Request text_request;
        Render_Image_Request image_request;
    };
};


struct Render_Context
{
    M_Arena *arena;
    
    Shaders_Hash shaders_hash;
    Shader_Program shaders[ShaderKind_Count];
    
    Font fonts[FontKind_Count];
    Font_Kind active_font;
    
    u32 requests_count;
    Render_Request render_requests[65536];
    
    v2 screen;
    f32 pixels_per_meter;
    
    Coordinate_Type active_coordinates_type;
};

#endif //GAME_RENDER_H
