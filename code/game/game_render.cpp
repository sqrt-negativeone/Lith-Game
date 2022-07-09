
#define MAX_Z Meter(10.0f)

internal void
InitRenderer(Render_Context *render_context)
{
    // NOTE(fakhri): load shaders
    for (Shader_Kind shader_kind = ShaderKind_None;
         shader_kind < ShaderKind_Count;
         ++shader_kind)
    {
        LoadShader(render_context, shader_kind);
    }
    
    // NOTE(fakhri): load fonts
    for (Font_Kind font_kind = FontKind_None;
         font_kind < FontKind_Count;
         ++font_kind)
    {
        LoadFont(render_context, os->permanent_arena, font_kind);
    }
    
    for (TextureID texture_id = TextureID_None;
         texture_id < TextureID_Count;
         ++texture_id)
    {
        LoadTexture(render_context, texture_id);
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
    // NOTE(fakhri): inverse the y
    screen_coords.y = render_context->screen.height - screen_coords.y;
    screen_coords -= render_context->camera_position;
    result.xy = screen_coords / render_context->pixels_per_meter;
    return result;
}

internal v2
SizeFromScreenCoordsToWorldCoords(Render_Context *render_context, v2 size)
{
    f32 meter_per_pixel = 1.0f / render_context->pixels_per_meter;
    v2 result = meter_per_pixel * size;
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
        ++push_buffer->requests_count;
    }
    return result;
}

internal void
Render_ClearBuffer(Push_Buffer *push_buffer)
{
    push_buffer->size = 0;
    push_buffer->requests_count = 0;
};

internal void
Render_PushClear(Render_Context *render_context, v4 color)
{
    // NOTE(fakhri): clear all render request that came before this
    Render_ClearBuffer(render_context->push_buffer);
    RenderRequest_Clear *clear_request = Render_PushRequest(render_context->push_buffer, RenderRequest_Clear);
    clear_request->header.kind = RenderKind_Clear;
    clear_request->header.z = -MAX_Z;
    clear_request->header.offset_to_next = sizeof(RenderRequest_Clear);
    clear_request->color = color;
}

internal void
Render_PushQuad(Render_Context *render_context, v3 pos, v2 size_in_meter, v4 color, Coordinate_Type coord_type, v3 orientation = Vec3(0, 0, 0))
{
    Assert(coord_type < CoordinateType_Count);
    if (coord_type == CoordinateType_World)
    {
        pos = ScreenCoordsFromWorldCoords(render_context, pos);
    }
    
    RenderRequest_Quad *quad_request = Render_PushRequest(render_context->push_buffer, RenderRequest_Quad);
    
    quad_request->header.kind = RenderKind_Quad;
    quad_request->header.z = pos.z;
    quad_request->header.offset_to_next = sizeof(RenderRequest_Quad);
    quad_request->screen_coords = pos.xy;
    quad_request->size = render_context->pixels_per_meter * size_in_meter;
    quad_request->color = color;
    quad_request->orientation = orientation;
    
}

internal void
Render_PushImage(Render_Context *render_context, Texture2D texture,
                 v3 pos, v2 size, 
                 Coordinate_Type coord_type, b32 flip_y = false,
                 v3 orientation = Vec3(0, 0, 0), b32 is_size_in_meter = true, v4 color = Vec4(1, 1, 1, 1), v4 src = Vec4(0, 0, 1, 1))
{
    Assert(coord_type < CoordinateType_Count);
    if (coord_type == CoordinateType_World)
    {
        pos = ScreenCoordsFromWorldCoords(render_context, pos);
    }
    
    RenderRequest_Image *image_request = Render_PushRequest(render_context->push_buffer, RenderRequest_Image);
    
    image_request->header.kind = RenderKind_Image;
    image_request->header.z = pos.z;
    image_request->header.offset_to_next = sizeof(RenderRequest_Image);
    image_request->screen_coords = pos.xy;
    image_request->texture = texture;
    image_request->flip_y = flip_y;
    image_request->size = is_size_in_meter ? render_context->pixels_per_meter * size : size;
    image_request->orientation = orientation;
    image_request->color = color;
    image_request->src = src;
}

internal f32
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
    
    v2 curr_point = pos.xy;
    // NOTE(fakhri): render each character
    {
        for (u32 ch_index = 0;
             ch_index < text.len;
             ++ch_index)
        {
            u8 ch = text.str[ch_index];
            Assert(font->map_first <= ch && ch < font->map_opl);
            Glyph glyph = font->map[ch - font->map_first];
            
            v2 glyph_pos = curr_point + 0.5f * glyph.size + glyph.offset;
            pos.xy = glyph_pos;
            Render_PushImage(render_context, font->texture, 
                             pos, glyph.size, 
                             CoordinateType_Screen, 0,
                             Vec3(0, 0, 0), 0, color, glyph.src.compact_rect);
            
            curr_point.x += glyph.advance;
        }
    }
    if (coord_type == CoordinateType_World)
    {
        curr_point = WorldCoordsFromScreenCoords(render_context, curr_point).xy;
    }
    return curr_point.x;
}

internal Push_Buffer *
AllocatePushBuffer(M_Arena *arena)
{
    Push_Buffer *push_buffer = (Push_Buffer *)M_ArenaPushZero(arena, Megabytes(1));
    
    push_buffer->memory = push_buffer + sizeof(Push_Buffer);
    push_buffer->capacity = Megabytes(1) - sizeof(Push_Buffer);
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
        // TODO(fakhri): zoom level?
        f32 meters_in_screen_width = Meter(1.0f);
        render_context->pixels_per_meter = render_context->screen.width / meters_in_screen_width;
        
        // TODO(fakhri): aspect ratio
    }
    
    v2 camera_center = 0.5f * render_context->screen;
    
    // NOTE(fakhri): mouse influence should be zero at the cetner of the screen
    render_context->mouse_influence = 0.05f;
    v2 camera_offset = render_context->mouse_influence * (os->mouse_position - camera_center);
    
    render_context->camera_position = Vec2(camera_center.x - camera_offset.x, camera_center.y + camera_offset.y);
    
    render_context->push_buffer = AllocatePushBuffer(frame_arena);
}

internal RenderSortBuffer
MakeSortBuffer(M_Arena *arena, u32 count)
{
    RenderSortBuffer sort_buffer;
    sort_buffer.elements = PushArray(arena, RenderSortEntry, count);
    sort_buffer.count = count;
    return sort_buffer;
};

internal void
SortRenderRequests(M_Arena *arnea, RenderSortBuffer *sort_buffer)
{
    for (u32 i = 0;
         i < sort_buffer->count;
         ++i)
    {
        f32 min_z = sort_buffer->elements[i].z;
        u32 index_min = i;
        for (u32 j = i + 1;
             j < sort_buffer->count;
             ++j)
        {
            f32 z_j = sort_buffer->elements[j].z;
            if (z_j < min_z)
            {
                min_z = z_j;
                index_min = j;
            }
        }
        
        RenderSortEntry tmp = sort_buffer->elements[i];
        sort_buffer->elements[i] = sort_buffer->elements[index_min];
        sort_buffer->elements[index_min] = tmp;
    }
}

internal void
Render_End(Render_Context *render_context)
{
    m4 ortho_projection = Orthographic(0.0f, render_context->screen.width, 
                                       render_context->screen.height, 0.0f,
                                       -MAX_Z, MAX_Z);
    
    Push_Buffer *push_buffer = render_context->push_buffer;
    RenderSortBuffer sort_buffer = MakeSortBuffer(render_context->frame_arena, push_buffer->requests_count);
    
    for(u32 offset = 0, entry_index = 0;
        offset < push_buffer->size, entry_index < sort_buffer.count;
        ++entry_index)
    {
        RenderSortEntry *sort_entry = sort_buffer.elements + entry_index;
        RenderRequest_Header *header = (RenderRequest_Header *)((byte_ptr)push_buffer->memory + offset);
        sort_entry->z = header->z;
        sort_entry->offset = offset;
        offset += header->offset_to_next;
        ++sort_entry;
    }
    
    SortRenderRequests(render_context->frame_arena, &sort_buffer);
    
    for(u32 entry_index = 0;
        entry_index < sort_buffer.count;
        ++entry_index)
    {
        RenderSortEntry *sort_entry = sort_buffer.elements + entry_index;
        RenderRequest_Header *header = (RenderRequest_Header *)((byte_ptr)push_buffer->memory + sort_entry->offset);
        switch(header->kind)
        {
            case RenderKind_Quad:
            {
                RenderRequest_Quad *quad_request = (RenderRequest_Quad *)header;
                v3 position = Vec3(quad_request->screen_coords, header->z);
                m4 trans = Translate(position);
                m4 scale = Scale(Vec3(quad_request->size, 1.0f));
                m4 rotat = (Rotate(quad_request->orientation.z, Vec3(0,0,1)) * 
                            Rotate(quad_request->orientation.y, Vec3(0,1,0)));
                
                m4 model = trans * rotat * scale;
                
                Shader_Program *program = render_context->shaders + ShaderKind_Quad;
                OpenGL_DrawSolidQuad(program, ortho_projection, model, quad_request->color);
                
            } break;
            
            case RenderKind_Image:
            {
                RenderRequest_Image *image_request = (RenderRequest_Image *)header;
                
                v3 position = Vec3(image_request->screen_coords, header->z);
                m4 trans = Translate(position);
                m4 scale = Scale(Vec3(image_request->size, 1.0f));
                m4 rotat = (Rotate(image_request->orientation.z, Vec3(0,0,1)) * 
                            Rotate(image_request->orientation.y, Vec3(0,1,0)));
                
                m4 model = trans * rotat * scale;
                
                Shader_Program *program = render_context->shaders + ShaderKind_Texture;
                OpenGL_DrawImage(program, image_request->texture, image_request->flip_y, ortho_projection, model, image_request->src, image_request->color);
                
            } break;
            
            case RenderKind_Clear:
            {
                RenderRequest_Clear *clear_request = (RenderRequest_Clear *)header;
                OpenGL_Clear(clear_request->color);
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
