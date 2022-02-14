/* date = January 29th 2022 11:29 am */

#ifndef RENDERER_H
#define RENDERER_H

#define MAX_UNITS_PER_X (100)

struct Character
{
    GLuint texture_id;  // ID handle of the glyph texture
    iv2    size;       // Size of glyph
    iv2    bearing;    // Offset from baseline to left/top of glyph
    u32    advance;    // Offset to advance to next glyph
};

struct Font
{
#define CHARACTERS_COUNT 128
    Character characters[CHARACTERS_COUNT];
};

struct Rendering_Context
{
    GLuint quad_shader;
    GLuint texture_shader;
    GLuint text_shader;
    
    GLuint quad_vao;
    GLuint texture_vao;
    GLuint text_vao;
    
    GLuint quad_vbo;
    GLuint texture_vbo;
    GLuint text_vbo;
    
    v2 screen;
    
    GLuint test_2d_texture;
    
    Font arial_font;
    Font *active_font;
    
    f32 normalized_width_unit_per_world_unit;
    f32 aspect_ratio;
};

#endif //RENDERER_H
