
#ifdef internal
// NOTE(fakhri): ft2 uses internal as a name for variable, *sigh*
#undef internal
#include "ft2build.h"
#include FT_FREETYPE_H
#define internal static
#endif

internal void
LoadGlyphsFromFont(FT_Library ft, Font *font, s8 font_path, u32 font_size)
{
    FT_Face face;
    if (!FT_New_Face(ft, font_path.str, 0, &face))
    {
        FT_Set_Pixel_Sizes(face, 0, font_size);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        for (u8 c = 0; c < CHARACTERS_COUNT; ++c)
        {
            if (!FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                GLuint texture;
                glGenTextures(1, &texture);
                glBindTexture(GL_TEXTURE_2D, texture);
                glTexImage2D(
                             GL_TEXTURE_2D,
                             0,
                             GL_RED,
                             face->glyph->bitmap.width,
                             face->glyph->bitmap.rows,
                             0,
                             GL_RED,
                             GL_UNSIGNED_BYTE,
                             face->glyph->bitmap.buffer
                             );
                // set texture options
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                // now store character for later use
                Character character = {
                    texture, 
                    iv2{(i32)face->glyph->bitmap.width, (i32)face->glyph->bitmap.rows},
                    iv2{face->glyph->bitmap_left, face->glyph->bitmap_top},
                    (u32)face->glyph->advance.x
                };
                font->characters[c] = character;
            }
            else
            {
                LogError("couldn't load character %c from font %s", c, font_path.str);
            }
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        FT_Done_Face(face);
    }
    else
    {
        LogError("Couldn't load font %s", font_path.str);
    }
}

internal void
UpdateScreenSize(Rendering_Context *rendering_context)
{
    v2 current_screen = vec2iv(os->window_size);
    
    if (rendering_context->screen != current_screen)
    {
        rendering_context->screen = current_screen;
        m4 ortho_projection = m4_orthographic(0.0f, rendering_context->screen.width, 
                                              rendering_context->screen.height, 0.0f,
                                              0.f, 100.f);
        
        // NOTE(fakhri): update projection matrices for all the shader that user it
        GLint projection_location;
        
        glUseProgram(rendering_context->quad_shader);
        projection_location = glGetUniformLocation(rendering_context->quad_shader, "projection");
        glUniformMatrix4fv(projection_location, 1, GL_FALSE, (f32*)&ortho_projection);
        
        glUseProgram(rendering_context->texture_shader);
        projection_location = glGetUniformLocation(rendering_context->texture_shader, "projection");
        glUniformMatrix4fv(projection_location, 1, GL_FALSE, (f32*)&ortho_projection);
        
        glUseProgram(rendering_context->text_shader);
        projection_location = glGetUniformLocation(rendering_context->text_shader, "projection");
        glUniformMatrix4fv(projection_location, 1, GL_FALSE, (f32*)&ortho_projection);
        glUseProgram(0);
        
        // NOTE(fakhri): adjust view port
        glViewport(0, 0, os->window_size.width, os->window_size.height);
    }
}

internal void
InitRenderer(Rendering_Context *rendering_context)
{
    // NOTE(fakhri): load shaders
    InitShaderProgram(&rendering_context->quad_shader, S8Lit("shaders/quad_shader.glsl"));
    InitShaderProgram(&rendering_context->texture_shader, S8Lit("shaders/texture_shader.glsl"));
    InitShaderProgram(&rendering_context->text_shader, S8Lit("shaders/text_shader.glsl"));
    
    // NOTE(fakhri): quad vertex buffers
    {
        f32 quad_vertices[] = {
            -0.5f, +0.5f,
            +0.5f, +0.5f,
            +0.5f, -0.5f,
            
            -0.5f, +0.5f,
            +0.5f, -0.5f,
            -0.5f, -0.5f,
        };
        
        glGenVertexArrays(1, &rendering_context->quad_vao);
        glBindVertexArray(rendering_context->quad_vao);
        
        glGenBuffers(1, &rendering_context->quad_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, rendering_context->quad_vbo);
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), (void*)0);
        glEnableVertexAttribArray(0);
    }
    
    // NOTE(fakhri): texture vertex buffers
    {
        f32 texture_vertices[] = {
            // position         texture
            -0.5f, -0.5f, 	 0.0f, 0.0f,
            +0.5f,  0.5f, 	 1.0f, 1.0f,
            -0.5f,  0.5f, 	 0.0f, 1.0f,
            
            -0.5f, -0.5f, 	 0.0f, 0.0f,
            +0.5f, -0.5f, 	 1.0f, 0.0f,
            +0.5f,  0.5f, 	 1.0f, 1.0f
        };
        
        glGenVertexArrays(1, &rendering_context->texture_vao);
        glBindVertexArray(rendering_context->texture_vao);
        
        glGenBuffers(1, &rendering_context->texture_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, rendering_context->texture_vbo);
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(texture_vertices), texture_vertices, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void *)0);
    }
    
    // NOTE(fakhri): text vertex buffers
    {
        glGenVertexArrays(1, &rendering_context->text_vao);
        glGenBuffers(1, &rendering_context->text_vbo);
        glBindVertexArray(rendering_context->text_vao);
        glBindBuffer(GL_ARRAY_BUFFER, rendering_context->text_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(v4) * 6, 0, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(v4), 0);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
    
    // NOTE(fakhri): load textures
    {
        s8 image_path = S8Lit("images/take_my_money.jpg");
        
        glGenTextures(1, &rendering_context->test_2d_texture);
        glBindTexture(GL_TEXTURE_2D, rendering_context->test_2d_texture);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        i32 nb_channels, tex_width, tex_height;
        void* data = stbi_load(image_path.str, &tex_width, &tex_height, &nb_channels, 0);
        if (data)
        {
            GLenum format = 0;
            switch(nb_channels)
            {
                case 1: format = GL_RED; break;
                case 3: format = GL_RGB; break;
                case 4: format = GL_RGBA; break;
                default: LogWarning("channels count %d not handled when loading image %s", nb_channels, image_path.str);
            }
            
            glTexImage2D(GL_TEXTURE_2D, 0, format, tex_width, tex_height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
            
        }
        else
        {
            LogWarning("couldn't load image %s", image_path.str);
        }
        
        // NOTE(fakhri): load fonts
        FT_Library ft;
        if (!FT_Init_FreeType(&ft))
        {
            s8 font_path = S8Lit("fonts/arial.ttf");
            LoadGlyphsFromFont(ft, &rendering_context->arial_font, font_path, 48);
            FT_Done_FreeType(ft);
        }
        else
        {
            LogError("Couldn't initialize free type library");
        }
    }
}

internal void
DebugDrawQuadScreenCoord(Rendering_Context *rendering_context, v2 pos, v2 size, v3 color, f32 z_rotation = 0.f)
{
    m4 trans = m4_translate(vec3(pos, 0.f));
    m4 rotat = m4_translate(vec3(-size / 2, 0.f)) * m4_rotate(-z_rotation, v3{0.f, 0.f, 1.f}) * m4_translate(vec3(size / 2, 0.f));
    m4 scale = m4_scale(vec3(size, 1.0f));
    m4 model = trans * rotat * scale;
    
    glUseProgram(rendering_context->quad_shader);
    GLint model_location = glGetUniformLocation(rendering_context->quad_shader, "model");
    glUniformMatrix4fv(model_location, 1, GL_FALSE, (f32*)&model);
    GLint color_location = glGetUniformLocation(rendering_context->quad_shader, "color");
    glUniform4f(color_location, color.r, color.g, color.b, 1.0f);
    
    glBindVertexArray(rendering_context->quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glUseProgram(0);
}

internal
void DebugDrawQuad(Rendering_Context *rendering_context, v2 pos, v2 size, v3 color, f32 z_rotation = 0.f)
{
    f32 window_width = (f32)os->window_size.width;
    f32 window_height = (f32)os->window_size.height;
    
    pos = v2{pos.x + rendering_context->screen.width / 2, rendering_context->screen.height / 2 - pos.y};
    DebugDrawQuadScreenCoord(rendering_context, pos, size, color, z_rotation);
}

internal
void DebugDrawTexture(Rendering_Context *rendering_context, GLuint tex, v2 pos, v2 size)
{
    f32 window_width = (f32)os->window_size.width;
    f32 window_height = (f32)os->window_size.height;
    
    // NOTE(fakhri): the origin at the center of the screen
    pos = v2{pos.x + window_width / 2, window_height / 2 - pos.y};
    m4 trans = m4_translate(vec3(pos, 0.f));
    m4 scale = m4_scale(vec3(size, 1.0f));
    m4 model = trans * scale;
    
    glUseProgram(rendering_context->texture_shader);
    GLint model_location = glGetUniformLocation(rendering_context->quad_shader, "model");
    glUniformMatrix4fv(model_location, 1, GL_FALSE, (f32*)&model);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glBindVertexArray(rendering_context->texture_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glUseProgram(0);
}

internal
void ChangeActiveFont(Rendering_Context *rendering_context, Font *new_font)
{
    rendering_context->active_font = new_font;
}

internal
f32 GetActiveFontWidth(Rendering_Context *rendering_context, s8 text, f32 scale = 1.0f)
{
    f32 text_width = 0;
    if (rendering_context->active_font)
    {
        Font *font = rendering_context->active_font;
        for (u32 character_index = 0;
             character_index < text.size;
             ++character_index)
        {
            Character ch = font->characters[text.str[character_index]];
            text_width += (ch.advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
        }
    }
    else
    {
        LogError("active font is null");
        BreakDebugger();
    }
    return text_width;
}


internal
f32 GetActiveFontHeight(Rendering_Context *rendering_context, f32 scale = 1.0f)
{
    f32 text_height = 0;
    if (rendering_context->active_font)
    {
        Font *font = rendering_context->active_font;
        text_height = (f32)font->characters['H'].bearing.y;
        text_height *= scale;
    }
    else
    {
        LogError("active font is null");
        BreakDebugger();
    }
    return text_height;
}

internal
void DebugDrawText(Rendering_Context *rendering_context, s8 text, v2 pos, v3 text_color, f32 scale = 1.0f)
{
    if (rendering_context->active_font)
    {
        Font *font = rendering_context->active_font;
        glUseProgram(rendering_context->text_shader);
        GLuint color_location = glGetUniformLocation(rendering_context->text_shader, "text_color");
        glUniform3fv(color_location, 1, (f32 *)&text_color);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(rendering_context->text_vao);
        
        f32 half_text_height = 0.5f * GetActiveFontHeight(rendering_context, scale);
        
        // NOTE(fakhri): adjust text position
        pos.x -= 0.5f * GetActiveFontWidth(rendering_context, text, scale);
        pos.y += font->characters['H'].bearing.y * scale - half_text_height;
        
        for (u32 character_index = 0;
             character_index < text.size;
             ++character_index)
        {
            Character ch = font->characters[text.str[character_index]];
            
            f32 xpos = pos.x + ch.bearing.x * scale;
            
            //f32 yoffset = half_text_height;
            //f32 ypos = pos.y + (font->characters['H'].bearing.y - ch.bearing.y) * scale - yoffset;
            
            f32 ypos = pos.y  - ch.bearing.y * scale;
            
            float w = ch.size.x * scale;
            float h = ch.size.y * scale;
            
            float vertices[] = {
                xpos, 		ypos + h, 	0.0f, 1.0f,
                xpos + w, 	ypos, 		1.0f, 0.0f,
                xpos, 		ypos, 		0.0f, 0.0f,
                
                xpos, 		ypos + h, 	0.0, 1.0f,
                xpos + w, 	ypos + h, 	1.0f, 1.0f,
                xpos + w, 	ypos, 		1.0f, 0.0f
            };
            
            // render glyph texture over quad
            glBindTexture(GL_TEXTURE_2D, ch.texture_id);
            // update content of VBO memory
            glBindBuffer(GL_ARRAY_BUFFER, rendering_context->text_vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            // render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
            // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            pos.x += (ch.advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else
    {
        LogError("active font is null");
        BreakDebugger();
    }
}

internal
void DebugDrawTextWorldCoord(Rendering_Context *rendering_context, s8 text, v2 pos, v3 text_color, f32 scale = 1.0f)
{
    v2 half_screen = 0.5f * rendering_context->screen;
    v2 screen_pos = vec2(pos.x + half_screen.x,
                         -pos.y + half_screen.y);
    //v2 screen_pos = pos + 0.5f * rendering_context->screen;
    DebugDrawText(rendering_context, text, screen_pos, text_color, scale);
}

internal void
DebugDrawRectangle(Rendering_Context *rendering_context, Rectangle2D rectange, v3 color)
{
    // TODO(fakhri): render rectangle
}