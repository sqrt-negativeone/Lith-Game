
internal void
UpdateScreenSize(Game_State *game_state)
{
    v2 new_screen = Vec2f32FromVec2i32(os->window_size);
    if (new_screen != game_state->screen)
    {
        game_state->screen = new_screen;
        
        m4 ortho_projection = Orthographic(0.0f, game_state->screen.width, 
                                           game_state->screen.height, 0.0f,
                                           -100.f, 100.f);
        
        for(u32 program_index = 1;
            program_index < ShaderKind_Count;
            ++program_index)
        {
            Shader_Program *program = game_state->shaders + program_index;
            glUseProgram(program->id);
            
            glUniformMatrix4fv(glGetUniformLocation(program->id, "projection"), 1, GL_FALSE, (f32*)&ortho_projection);
            
        }
        
        glUseProgram(0);
        // TODO(fakhri): aspect ratio
    }
}

internal void
LoadFrenshSuitedDeck(Frensh_Suited_Cards_Texture *deck_textures)
{
    deck_textures->card_frame_texture    = LoadTexture(Str8Lit("data/images/card_frame.png"));
    deck_textures->card_back_texture     = LoadTexture(Str8Lit("data/images/card_back.png"));
    deck_textures->clovers_up            = LoadTexture(Str8Lit("data/images/clovers_up.png"));
    deck_textures->hearts_up             = LoadTexture(Str8Lit("data/images/hearts_up.png"));
    deck_textures->pikes_up              = LoadTexture(Str8Lit("data/images/pikes_up.png"));
    deck_textures->clovers_down          = LoadTexture(Str8Lit("data/images/clovers_down.png"));
    deck_textures->hearts_down           = LoadTexture(Str8Lit("data/images/hearts_down.png"));
    deck_textures->pikes_down            = LoadTexture(Str8Lit("data/images/pikes_down.png"));
    deck_textures->tiles                 = LoadTexture(Str8Lit("data/images/tiles.png"));
    
    for (u32 number_index = Card_Number_2;
         number_index <= Card_Number_10;
         ++number_index)
    {
        u32 number = number_index + 1;
        char buffer[50];
        sprintf(buffer, "data/images/%d_black_up.png", number);
        deck_textures->black_numbers_up[number_index]    = LoadTexture(Str8C(buffer));
        sprintf(buffer, "data/images/%d_black_down.png", number);
        deck_textures->black_numbers_down[number_index]  = LoadTexture(Str8C(buffer));
        sprintf(buffer, "data/images/%d_red_up.png", number);
        deck_textures->red_numbers_up[number_index]      = LoadTexture(Str8C(buffer));
        sprintf(buffer, "data/images/%d_red_down.png", number);
        deck_textures->red_numbers_down[number_index]    = LoadTexture(Str8C(buffer));
    }
    
    deck_textures->black_numbers_up[Card_Number_Ace]   = LoadTexture(Str8Lit("data/images/A_black_up.png"));
    deck_textures->black_numbers_down[Card_Number_Ace] = LoadTexture(Str8Lit("data/images/A_black_down.png"));
    deck_textures->red_numbers_up[Card_Number_Ace]     = LoadTexture(Str8Lit("data/images/A_red_up.png"));
    deck_textures->red_numbers_down[Card_Number_Ace]   = LoadTexture(Str8Lit("data/images/A_red_down.png"));
    
    deck_textures->black_numbers_up[Card_Number_Jack]   = LoadTexture(Str8Lit("data/images/J_black_up.png"));
    deck_textures->black_numbers_down[Card_Number_Jack] = LoadTexture(Str8Lit("data/images/J_black_down.png"));
    deck_textures->red_numbers_up[Card_Number_Jack]     = LoadTexture(Str8Lit("data/images/J_red_up.png"));
    deck_textures->red_numbers_down[Card_Number_Jack]   = LoadTexture(Str8Lit("data/images/J_red_down.png"));
    
    deck_textures->black_numbers_up[Card_Number_Queen]   = LoadTexture(Str8Lit("data/images/Q_black_up.png"));
    deck_textures->black_numbers_down[Card_Number_Queen] = LoadTexture(Str8Lit("data/images/Q_black_down.png"));
    deck_textures->red_numbers_up[Card_Number_Queen]     = LoadTexture(Str8Lit("data/images/Q_red_up.png"));
    deck_textures->red_numbers_down[Card_Number_Queen]   = LoadTexture(Str8Lit("data/images/Q_red_down.png"));
    
    deck_textures->black_numbers_up[Card_Number_King]   = LoadTexture(Str8Lit("data/images/K_black_up.png"));
    deck_textures->black_numbers_down[Card_Number_King] = LoadTexture(Str8Lit("data/images/K_black_down.png"));
    deck_textures->red_numbers_up[Card_Number_King]     = LoadTexture(Str8Lit("data/images/K_red_up.png"));
    deck_textures->red_numbers_down[Card_Number_King]   = LoadTexture(Str8Lit("data/images/K_red_down.png"));
    
    deck_textures->jacks[Category_Clovers]  = LoadTexture(Str8Lit("data/images/jack_clovers.png"));
    deck_textures->queens[Category_Clovers] = LoadTexture(Str8Lit("data/images/queen_clovers.png"));
    deck_textures->kings[Category_Clovers]  = LoadTexture(Str8Lit("data/images/king_clovers.png"));
    
    deck_textures->jacks[Category_Hearts]  = LoadTexture(Str8Lit("data/images/jack_hearts.png"));
    deck_textures->queens[Category_Hearts] = LoadTexture(Str8Lit("data/images/queen_hearts.png"));
    deck_textures->kings[Category_Hearts]  = LoadTexture(Str8Lit("data/images/king_hearts.png"));
    
    deck_textures->jacks[Category_Pikes]  = LoadTexture(Str8Lit("data/images/jack_pikes.png"));
    deck_textures->queens[Category_Pikes] = LoadTexture(Str8Lit("data/images/queen_pikes.png"));
    deck_textures->kings[Category_Pikes]  = LoadTexture(Str8Lit("data/images/king_pikes.png"));
    
    deck_textures->jacks[Category_Tiles]  = LoadTexture(Str8Lit("data/images/jack_tiles.png"));
    deck_textures->queens[Category_Tiles] = LoadTexture(Str8Lit("data/images/queen_tiles.png"));
    deck_textures->kings[Category_Tiles]  = LoadTexture(Str8Lit("data/images/king_tiles.png"));
    
}

internal void
InitRenderer(Game_State *game_state)
{
    
}

internal void
DrawQuadScreenCoord(Game_State *game_state, v3 pos, v2 size, v3 color, f32 y_angle = 0.0f)
{
    m4 trans = Translate(pos);
    m4 scale = Scale(Vec3(size, 1.0f));
    m4 rotat = Rotate(y_angle, Vec3(0,1,0));
    
    m4 model = trans * rotat * scale;
    
    Shader_ID quad_shader = game_state->shaders[ShaderKind_Quad].id;
    glUseProgram(quad_shader);
    GLint model_location = glGetUniformLocation(quad_shader, "model");
    glUniformMatrix4fv(model_location, 1, GL_FALSE, (f32*)&model);
    GLint color_location = glGetUniformLocation(quad_shader, "color");
    glUniform4f(color_location, color.r, color.g, color.b, 1.0f);
    
    glBindVertexArray(game_state->shaders[ShaderKind_Quad].vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glUseProgram(0);
    glBindVertexArray(0);
    
}

internal void
DrawQuadNormalizedCoord(Game_State *game_state, v3 pos, v2 size, v3 color, f32 y_angle = 0.0f)
{
    f32 window_width = (f32)os->window_size.width;
    f32 window_height = (f32)os->window_size.height;
    pos.xy = Vec2(pos.x * window_width, pos.y * window_height);
    size = Vec2(size.width * window_width, size.height * window_height);
    
    DrawQuadScreenCoord(game_state, pos, size, color, y_angle);
}

internal void
DrawQuadWorldCoord(Game_State *game_state, v3 pos, v2 size, v3 color, f32 y_angle = 0.0f)
{
    f32 x_correction = game_state->normalized_width_unit_per_world_unit;
    f32 y_correction = (1.0f / game_state->aspect_ratio) * x_correction;
    pos.xy = Vec2(pos.x * x_correction,
                  1.0f - pos.y * y_correction);
    size = Vec2(size.x * x_correction,
                size.y * y_correction);
    
    DrawQuadNormalizedCoord(game_state, pos, size, color, y_angle);
}

internal void
DrawTextureScreenCoord(Game_State *game_state, Texture2D tex, v3 pos, v2 size, f32 y_angle = 0.0f, v3 rotation_axe = Vec3(0, 1, 0))
{
    m4 trans = Translate(pos);
    m4 scale = Scale(Vec3(size, 1.0f));
    m4 rotat = Rotate(y_angle, rotation_axe);
    
    m4 model = trans * rotat * scale;
    
    glUseProgram(game_state->shaders[ShaderKind_Texture].id);
    GLint model_location = glGetUniformLocation(game_state->shaders[ShaderKind_Texture].id, "model");
    glUniformMatrix4fv(model_location, 1, GL_FALSE, (f32*)&model);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex.id);
    glBindVertexArray(game_state->shaders[ShaderKind_Texture].vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glUseProgram(0);
}

internal void 
DrawTextureNormalizedCoord(Game_State *game_state, Texture2D tex, v3 pos, v2 size, f32 y_angle = 0.0f)
{
    f32 window_width = (f32)os->window_size.width;
    f32 window_height = (f32)os->window_size.height;
    pos.xy = Vec2(pos.x * window_width, pos.y * window_height);
    size = Vec2(size.width * window_width, size.height * window_height);
    
    DrawTextureScreenCoord(game_state, tex, pos, size, y_angle);
}

internal void
DrawTextureWorldCoord(Game_State *game_state, Texture2D tex, v3 pos, v2 size, f32 y_angle = 0.0f)
{
    f32 x_correction = game_state->normalized_width_unit_per_world_unit;
    f32 y_correction = (1.0f / game_state->aspect_ratio) * x_correction;
    pos.xy = Vec2(pos.x * x_correction,
                  1.0f - pos.y * y_correction);
    size = Vec2(size.x * x_correction,
                size.y * y_correction);
    
    DrawTextureNormalizedCoord(game_state, tex, pos, size, y_angle);
}

internal void
DrawTextScreenCoord(Game_State *game_state, String8 string, v3 pos, v3 color, Font_Kind font_kind = FontKind_None)
{
    // NOTE(fakhri): select the font
    Font *font = 0;
    if (font_kind == FontKind_None)
    {
        Assert(game_state->active_font);
        font = game_state->active_font;
    }
    else
    {
        Assert(font_kind < FontKind_Count);
        font = game_state->fonts + font_kind;
    }
    
    if (string.len > 1)
    {
        int break_here = 1;
    }
    
    f32 text_width = GetFontWidth(font, string);
    pos.x -= 0.5f * text_width;
    Shader_Program *program = game_state->shaders + ShaderKind_Font;
    
    glUseProgram(program->id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font->texture.id);
    glBindVertexArray(program->vao);
    glUniform3f(glGetUniformLocation(program->id, "color"), color.r, color.g, color.b);
    
    // NOTE(fakhri): render the text
    {
        v2 current_point = pos.xy;
        for (u32 ch_index = 0;
             ch_index < string.len;
             ++ch_index)
        {
            u8 ch = string.str[ch_index];
            Assert(font->map_first <= ch && ch < font->map_opl);
            Glyph glyph = font->map[ch - font->map_first];
            
            v2 glyph_pos = current_point + 0.5f * glyph.size + glyph.offset;
            m4 trans = Translate(Vec3(glyph_pos, pos.z));
            m4 scale = Scale(Vec3(glyph.size, 1.0f));
            m4 model = trans * scale;
            
            glUniformMatrix4fv(glGetUniformLocation(program->id, "model"), 1, GL_FALSE, (f32*)&model);
            glUniform4fv(glGetUniformLocation(program->id, "glyph_src"), 1, (f32 *)&glyph.src);
            
            glDrawArrays(GL_TRIANGLES, 0, 6);
            
            current_point.x += glyph.advance;
        }
    }
    
    glBindVertexArray(0);
    glUseProgram(0);
    
}

internal void
DrawTextNormalizedCoord(Game_State *game_state, String8 text, v3 pos, v3 text_color)
{
    f32 window_width = (f32)os->window_size.width;
    f32 window_height = (f32)os->window_size.height;
    pos.xy = v2{pos.x * window_width, pos.y * window_height};
    DrawTextScreenCoord(game_state, text, pos, text_color);
}

internal void
DrawTextWorldCoord(Game_State *game_state, String8 string, v3 pos, v3 color, Font_Kind font_kind = FontKind_None)
{
    f32 x_correction = game_state->normalized_width_unit_per_world_unit;
    f32 y_correction = (1.0f / game_state->aspect_ratio) * x_correction;
    pos.xy = Vec2(pos.x * x_correction,
                  1.0f - pos.y * y_correction);
    
    DrawTextNormalizedCoord(game_state, string, pos, color);
}

internal void
DrawTextScreenCoord(Game_State *game_state, String8 string, v2 pos, v3 color, Font_Kind font_kind = FontKind_None)
{
    DrawTextScreenCoord(game_state, string, Vec3(pos, 0), color, font_kind);
}

internal void
DrawTextWorldCoord(Game_State *game_state, String8 string, v2 pos, v3 color, Font_Kind font_kind = FontKind_None)
{
    DrawTextWorldCoord(game_state, string, Vec3(pos, 0), color, font_kind);
}

internal inline v2
ScreenToWorldCoord(Game_State *game_state, v2 pos)
{
    pos.x *= 1.0f / game_state->screen.width;
    pos.y *= 1.0f / game_state->screen.height;
    
    f32 x_correction = (1.0f / game_state->normalized_width_unit_per_world_unit);
    f32 y_correction = x_correction * game_state->aspect_ratio;
    
    pos.x = pos.x *  x_correction;
    pos.y = (1.0f - pos.y) * y_correction;
    
    return pos;
}

internal inline v2
NormalizedToScreenCoords(Game_State *game_state, v2 pos)
{
    f32 window_width  = game_state->screen.width;
    f32 window_height = game_state->screen.height;
    pos = Vec2(pos.x * window_width, pos.y * window_height);
    return pos;
}

internal inline v2
WorldToNormalizedCoords(Game_State *game_state, v2 pos)
{
    f32 x_correction = game_state->normalized_width_unit_per_world_unit;
    f32 y_correction = (1.0f / game_state->aspect_ratio) * x_correction;
    pos = Vec2(pos.x * x_correction,
               1.0f - pos.y * y_correction);
    
}


internal inline v2
WorldToScreenCoords(Game_State *game_state, v2 pos)
{
    pos = WorldToNormalizedCoords(game_state, pos);
    pos = NormalizedToScreenCoords(game_state, pos);
    return pos;
}
