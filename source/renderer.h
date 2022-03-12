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
    f32 font_height;
};

struct Frensh_Suited_Cards_Texture
{
    GLuint card_frame_texture;
    GLuint card_back_texture;
    
    GLuint clovers_up;
    GLuint hearts_up;
    GLuint pikes_up;
    GLuint clovers_down;
    GLuint hearts_down;
    GLuint pikes_down;
    GLuint tiles;
    
    GLuint black_numbers_up[13];
    GLuint black_numbers_down[13];
    GLuint red_numbers_up[13];
    GLuint red_numbers_down[13];
    
    GLuint jacks[4];
    GLuint queens[4];
    GLuint kings[4];
};

struct Rendering_Context
{
    GLuint quad_shader;
    GLuint texture_shader;
    GLuint text_shader;
    
    GLuint quad_vao;
    GLuint texture_vao;
    GLuint text_vao;
    Frensh_Suited_Cards_Texture frensh_deck;
    GLuint quad_vbo;
    GLuint texture_vbo;
    GLuint text_vbo;
    
    v2 screen;
    
    
    Font arial_font;
    Font menu_title_font;
    Font menu_item_font;
    Font *active_font;
    
    f32 normalized_width_unit_per_world_unit;
    f32 aspect_ratio;
};

#endif //RENDERER_H
