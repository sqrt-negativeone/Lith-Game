
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
InitRenderer(Render_Context *render_context)
{
    // NOTE(fakhri): load shaders
    for (u32 shader_type_index = ShaderKind_None;
         shader_type_index < ShaderKind_Count;
         ++shader_type_index)
    {
        LoadShader(render_context, (Shader_Kind)shader_type_index);
    }
    
    // NOTE(fakhri): load fonts
    for (u32 font_index = FontKind_None;
         font_index < FontKind_Count;
         ++font_index)
    {
        LoadFont(render_context, os->permanent_arena, (Font_Kind)font_index);
    }
    
}

internal v3
ScreenCoordsFromWorldCoords(Render_Context *render_context, v3 world_coords)
{
    v3 result;
    
    result.xy = render_context->pixels_per_meter * world_coords.xy;
    result.z = world_coords.z;
    // NOTE(fakhri): world origin is at the center of the screen, and Y is up
    result.xy += 0.5f * render_context->screen;
    result.y = render_context->screen.height - result.y;
    return result;
}

internal v3
WorldCoordsFromScreenCoords(Render_Context *render_context, v2 screen_coords)
{
    v3 result = {};
    screen_coords.y = render_context->screen.height - screen_coords.y;
    screen_coords -= 0.5f * render_context->screen;
    result.xy = screen_coords / render_context->pixels_per_meter;
    return result;
}

internal void
Render_PushQuadRequest(Render_Context *render_context, v3 pos, v2 size_in_milimeter, v4 color, Coordinate_Type coord_type = CoordinateType_None, f32 y_angle = 0.0f)
{
    Assert(coord_type < CoordinateType_Count);
    Assert(render_context->requests_count < ArrayCount(render_context->render_requests));
    
    if (coord_type == CoordinateType_None)
    {
        Assert(render_context->active_coordinates_type > CoordinateType_None && render_context->active_coordinates_type < CoordinateType_Count);
        coord_type = render_context->active_coordinates_type;
    }
    
    if (coord_type == CoordinateType_World)
    {
        pos = ScreenCoordsFromWorldCoords(render_context, pos);
    }
    
    Render_Quad_Request quad_request = {};
    
    quad_request.size = render_context->pixels_per_meter * size_in_milimeter;
    quad_request.color = color;
    quad_request.y_angle = y_angle;
    
    Render_Request *request = render_context->render_requests + render_context->requests_count;
    ++render_context->requests_count;
    
    request->kind = RenderKind_Quad;
    request->screen_coords = pos;
    request->quad_request = quad_request;
}

internal void
Render_PushImageRequest(Render_Context *render_context, Texture2D texture, v3 pos, v2 size_in_milimeter, Coordinate_Type coord_type = CoordinateType_None, f32 y_angle = 0)
{
    Assert(coord_type < CoordinateType_Count);
    Assert(render_context->requests_count < ArrayCount(render_context->render_requests));
    
    if (coord_type == CoordinateType_None)
    {
        Assert(render_context->active_coordinates_type > CoordinateType_None && render_context->active_coordinates_type < CoordinateType_Count);
        coord_type = render_context->active_coordinates_type;
    }
    
    if (coord_type == CoordinateType_World)
    {
        pos = ScreenCoordsFromWorldCoords(render_context, pos);
    }
    Render_Image_Request image_request = {};
    
    image_request.texture = texture;
    image_request.size = render_context->pixels_per_meter * size_in_milimeter;
    image_request.y_angle = y_angle;
    
    Render_Request *request = render_context->render_requests + render_context->requests_count;
    ++render_context->requests_count;
    
    request->kind = RenderKind_Image;
    request->screen_coords = pos;
    request->image_request = image_request;
}

internal void
Render_PushTextRequest(Render_Context *render_context, String text, v3 pos, v4 color, Font_Kind font_to_use = FontKind_None, Coordinate_Type coord_type = CoordinateType_None)
{
    Assert(coord_type < CoordinateType_Count);
    Assert(render_context->requests_count < ArrayCount(render_context->render_requests));
    Assert(font_to_use > FontKind_None && font_to_use < FontKind_Count);
    
    if (coord_type == CoordinateType_None)
    {
        Assert(render_context->active_coordinates_type > CoordinateType_None && render_context->active_coordinates_type < CoordinateType_Count);
        coord_type = render_context->active_coordinates_type;
    }
    
    if (coord_type == CoordinateType_World)
    {
        pos = ScreenCoordsFromWorldCoords(render_context, pos);
    }
    
    if (font_to_use == FontKind_None)
    {
        Assert(render_context->active_font > FontKind_None && render_context->active_font < FontKind_Count);
        font_to_use = render_context->active_font;
    }
    
    pos.x -= 0.5f * GetFontWidth(render_context, font_to_use, text);
    Render_Text_Request text_request = {};
    
    text_request.text = PushStr8Copy(render_context->arena, text);
    text_request.font_to_use = font_to_use;
    text_request.color = color;
    
    Render_Request *request = render_context->render_requests + render_context->requests_count;
    ++render_context->requests_count;
    
    request->kind = RenderKind_Text;
    request->screen_coords = pos;
    request->text_request = text_request;
}

internal void
Render_Begin(Render_Context *render_context, M_Arena *arena)
{
    render_context->requests_count = 0;
    render_context->arena = arena;
    
    v2 new_screen = Vec2f32FromVec2i32(os->window_size);
    if (new_screen != render_context->screen)
    {
        render_context->screen = new_screen;
        
        f32 meters_in_half_screen_width = Meter(0.5f);
        render_context->pixels_per_meter = render_context->screen.width / (2 * meters_in_half_screen_width);
        
        // TODO(fakhri): aspect ratio
    }
    
}


internal void
Render_End(Render_Context *render_context)
{
    // TODO(fakhri): sort the requests from back to front
    
    m4 ortho_projection = Orthographic(0.0f, render_context->screen.width, 
                                       render_context->screen.height, 0.0f,
                                       -100.f, 100.f);
    
    for(u32 request_index = 0;
        request_index < render_context->requests_count;
        ++request_index)
    {
        Render_Request *request = render_context->render_requests + request_index;
        switch(request->kind)
        {
            case RenderKind_Quad:
            {
                m4 trans = Translate(request->screen_coords);
                m4 scale = Scale(Vec3(request->quad_request.size, 1.0f));
                m4 rotat = Rotate(request->quad_request.y_angle, Vec3(0,1,0));
                
                m4 model = trans * rotat * scale;
                
                Shader_Program *program = render_context->shaders + ShaderKind_Quad;
                glUseProgram(program->id);
                
                glUniformMatrix4fv(glGetUniformLocation(program->id, "projection"), 1, GL_FALSE, (f32*)&ortho_projection);
                glUniformMatrix4fv(glGetUniformLocation(program->id, "model"), 1, GL_FALSE, (f32*)&model);
                glUniform4fv(glGetUniformLocation(program->id, "color"), 1, (f32*)&request->quad_request.color);
                
                glBindVertexArray(program->vao);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                
            } break;
            case RenderKind_Image:
            {
                m4 trans = Translate(request->screen_coords);
                m4 scale = Scale(Vec3(request->image_request.size, 1.0f));
                request->image_request.y_angle = 0;
                m4 rotat = Rotate(request->image_request.y_angle, Vec3(0,1,0));
                
                m4 model = trans * rotat * scale;
                
                Shader_Program *program = render_context->shaders + ShaderKind_Texture;
                
                glUseProgram(program->id);
                
                glUniformMatrix4fv(glGetUniformLocation(program->id, "projection"), 1, GL_FALSE, (f32*)&ortho_projection);
                glUniformMatrix4fv(glGetUniformLocation(program->id, "model"), 1, GL_FALSE, (f32*)&model);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, request->image_request.texture.id);
                glBindVertexArray(program->vao);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                
            } break;
            case RenderKind_Text:
            {
                // NOTE(fakhri): select the font
                Assert(request->text_request.font_to_use < FontKind_Count);
                Font *font = render_context->fonts + request->text_request.font_to_use;
                
                
                Shader_Program *program = render_context->shaders + ShaderKind_Font;
                
                glUseProgram(program->id);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, font->texture.id);
                glBindVertexArray(program->vao);
                glUniformMatrix4fv(glGetUniformLocation(program->id, "projection"), 1, GL_FALSE, (f32*)&ortho_projection);
                glUniform4fv(glGetUniformLocation(program->id, "color"), 1, (f32 *)&request->text_request.color);
                
                // NOTE(fakhri): render the text
                {
                    v2 current_point = request->screen_coords.xy;
                    for (u32 ch_index = 0;
                         ch_index < request->text_request.text.len;
                         ++ch_index)
                    {
                        u8 ch = request->text_request.text.str[ch_index];
                        Assert(font->map_first <= ch && ch < font->map_opl);
                        Glyph glyph = font->map[ch - font->map_first];
                        
                        v2 glyph_pos = current_point + 0.5f * glyph.size + glyph.offset;
                        
                        m4 trans = Translate(Vec3(glyph_pos, request->screen_coords.z));
                        m4 scale = Scale(Vec3(glyph.size, 1.0f));
                        m4 model = trans * scale;
                        
                        glUniformMatrix4fv(glGetUniformLocation(program->id, "model"), 1, GL_FALSE, (f32*)&model);
                        glUniform4fv(glGetUniformLocation(program->id, "glyph_src"), 1, (f32 *)&glyph.src);
                        
                        glDrawArrays(GL_TRIANGLES, 0, 6);
                        
                        current_point.x += glyph.advance;
                    }
                }
            } break;
            default :
            {
                Assert(request->kind < RenderKind_Count);
                NotImplemented;
            } break;
        }
        
        glUseProgram(0);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
