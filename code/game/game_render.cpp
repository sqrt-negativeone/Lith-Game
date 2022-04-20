
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
    result.xy += render_context->camera_position;
    result.y = render_context->screen.height - result.y;
    return result;
}

internal v3
WorldCoordsFromScreenCoords(Render_Context *render_context, v2 screen_coords)
{
    v3 result = {};
    screen_coords.y = render_context->screen.height - screen_coords.y;
    screen_coords -= render_context->camera_position;
    result.xy = screen_coords / render_context->pixels_per_meter;
    return result;
}

#define Render_PushRequest(push_buffer, T) ((T *)_Render_PushRequest(push_buffer, sizeof(T)))

internal void *
_Render_PushRequest(Push_Buffer *push_buffer, u32 size)
{
    void *result = 0;
    Assert(push_buffer->size + size < push_buffer->capacity);
    if (push_buffer->size + size < push_buffer->capacity)
    {
        result = (u8 *)push_buffer->memory + push_buffer->size;
        push_buffer->size += size;
    }
    return result;
}


internal void
Render_PushQuad(Render_Context *render_context, v3 pos, v2 size_in_meter, v4 color, Coordinate_Type coord_type, f32 y_angle = 0.0f)
{
    Assert(coord_type < CoordinateType_Count);
    if (coord_type == CoordinateType_World)
    {
        pos = ScreenCoordsFromWorldCoords(render_context, pos);
    }
    
    Render_Quad_Request *quad_request = Render_PushRequest(render_context->push_buffer, Render_Quad_Request);
    
    quad_request->header.kind = RenderKind_Quad;
    quad_request->header.screen_coords = pos;
    quad_request->size = render_context->pixels_per_meter * size_in_meter;
    quad_request->color = color;
    quad_request->y_angle = y_angle;
    
}

internal void
Render_PushImage(Render_Context *render_context, Texture2D texture,
                 v3 pos, v2 size, 
                 Coordinate_Type coord_type, 
                 f32 y_angle = 0, b32 is_size_in_meter = true, v4 color = Vec4(1, 1, 1, 1), v4 src = Vec4(0, 0, 1, 1))
{
    Assert(coord_type < CoordinateType_Count);
    if (coord_type == CoordinateType_World)
    {
        pos = ScreenCoordsFromWorldCoords(render_context, pos);
    }
    
    Render_Image_Request *image_request = Render_PushRequest(render_context->push_buffer, Render_Image_Request);
    
    image_request->header.kind = RenderKind_Image;
    image_request->header.screen_coords = pos;
    image_request->texture = texture;
    image_request->size = is_size_in_meter ? render_context->pixels_per_meter * size : size;
    image_request->y_angle = y_angle;
    image_request->color = color;
    image_request->src = src;
}

internal void
Render_PushText(Render_Context *render_context, String text, v3 pos, v4 color, Coordinate_Type coord_type, Font_Kind font_to_use = FontKind_None)
{
    Assert(font_to_use < FontKind_Count);
    if (font_to_use == FontKind_None)
    {
        Assert(render_context->active_font > FontKind_None && render_context->active_font < FontKind_Count);
        font_to_use = render_context->active_font;
    }
    if (coord_type == CoordinateType_World)
    {
        pos = ScreenCoordsFromWorldCoords(render_context, pos);
    }
    pos.x -= 0.5f * GetFontWidth(render_context, font_to_use, text);
    
    Font *font = render_context->fonts + font_to_use;
    
    // NOTE(fakhri): render each character
    {
        v2 current_point = pos.xy;
        for (u32 ch_index = 0;
             ch_index < text.len;
             ++ch_index)
        {
            u8 ch = text.str[ch_index];
            Assert(font->map_first <= ch && ch < font->map_opl);
            Glyph glyph = font->map[ch - font->map_first];
            
            v2 glyph_pos = current_point + 0.5f * glyph.size + glyph.offset;
            pos.xy = glyph_pos;
            Render_PushImage(render_context, font->texture, 
                             pos, glyph.size, 
                             CoordinateType_Screen,
                             0, 0, color, glyph.src.compact_rect);
            
            current_point.x += glyph.advance;
        }
    }
}

internal Push_Buffer *
AllocatePushBuffer(M_Arena *arena)
{
    Push_Buffer *push_buffer = (Push_Buffer *)M_ArenaPushZero(arena, Megabytes(1));
    
    push_buffer->memory = push_buffer + sizeof(*push_buffer);
    push_buffer->capacity = Megabytes(1);
    push_buffer->size = 0;
    
    return push_buffer;
}

internal void
Render_Begin(Render_Context *render_context, M_Arena *frame_arena)
{
    render_context->frame_arena = frame_arena;
    
    v2 new_screen = Vec2f32FromVec2i32(os->window_size);
    if (new_screen != render_context->screen)
    {
        render_context->screen = new_screen;
        
        f32 meters_in_half_screen_width = Meter(0.5f);
        render_context->pixels_per_meter = render_context->screen.width / (2 * meters_in_half_screen_width);
        
        // TODO(fakhri): aspect ratio
    }
    
    v2 camera_center = 0.5f * render_context->screen;
    
    // NOTE(fakhri): mouse influence should be zero at the cetner of the camera
    v2 camera_offset = 0.05f * (os->mouse_position - camera_center);
    
    render_context->camera_position = Vec2(camera_center.x - camera_offset.x, camera_center.y + camera_offset.y);
    
    render_context->push_buffer = AllocatePushBuffer(frame_arena);
}


internal void
Render_End(Render_Context *render_context)
{
    // TODO(fakhri): sort the requests from back to front
    m4 ortho_projection = Orthographic(0.0f, render_context->screen.width, 
                                       render_context->screen.height, 0.0f,
                                       -100.f, 100.f);
    
    
    Push_Buffer *push_buffer = render_context->push_buffer;
    u8 *header_ptr = (u8*)render_context->push_buffer->memory;
    for(;
        header_ptr < (u8*)push_buffer->memory + push_buffer->size;
        )
    {
        Render_Request_Header *header = (Render_Request_Header *)header_ptr;
        switch(header->kind)
        {
            case RenderKind_Quad:
            {
                Render_Quad_Request *quad_request = (Render_Quad_Request *)header;
                m4 trans = Translate(header->screen_coords);
                m4 scale = Scale(Vec3(quad_request->size, 1.0f));
                m4 rotat = Rotate(quad_request->y_angle, Vec3(0,1,0));
                
                m4 model = trans * rotat * scale;
                
                Shader_Program *program = render_context->shaders + ShaderKind_Quad;
                OpenGL_DrawSolidQuad(program, ortho_projection, model, quad_request->color);
                
                header_ptr += sizeof(*quad_request);
            } break;
            
            case RenderKind_Image:
            {
                Render_Image_Request *image_request = (Render_Image_Request *)header;
                
                m4 trans = Translate(header->screen_coords);
                m4 scale = Scale(Vec3(image_request->size, 1.0f));
                m4 rotat = Rotate(image_request->y_angle, Vec3(0,1,0));
                m4 model = trans * rotat * scale;
                
                Shader_Program *program = render_context->shaders + ShaderKind_Texture;
                OpenGL_DrawImage(program, image_request->texture, ortho_projection, model, image_request->src, image_request->color);
                
                header_ptr += sizeof(*image_request);
            } break;
            default :
            {
                Assert(header->kind < RenderKind_Count);
                NotImplemented;
            } break;
        }
        
        glUseProgram(0);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
