
internal void
UI_InitColorScheme(UI_ColorScheme *color_scheme)
{
    // TODO(fakhri): choose some good colors
    // TODO(fakhri): maybe load this from a config file?
    
    *color_scheme = {};
    
    v3 white = vec3(1.0f, 1.0f, 1.0f);
    v3 none_white = 0.3f * white;
    v3 black = 0.f * white;
    v3 red = vec3(1.0f, 0.f, 0.f);
    v3 blue = vec3(78, 166, 206) / 255.f;
    
    color_scheme->label_color = white;
    color_scheme->label_color1 = white;
    color_scheme->label_color2 = 0.8f * white;
    
    // NOTE(fakhri): button colors
    color_scheme->button_text_color = 0.3f * white;
    color_scheme->button_text_active_color1 = 0.85f * blue;
    color_scheme->button_text_active_color2 = blue;
    color_scheme->button_text_shadow_color = black;
    
    // NOTE(fakhri): input field color
    color_scheme->input_field_background_color = 0.2f * white;
    color_scheme->input_field_background_active_color = 0.5f * white;
    
    color_scheme->input_field_text_color = color_scheme->input_field_background_active_color;
    color_scheme->input_field_text_active_color = color_scheme->input_field_background_color;
}

internal void
UI_Init(UI_Context *ui_context, M_Arena *arena)
{
    for (u32 buffer_index = 0;
         buffer_index < ArrayCount(ui_context->input_field_buffers);
         ++buffer_index)
    {
        Buffer *buffer = ui_context->input_field_buffers + buffer_index;
        *buffer = InitBuffer(arena, SERVER_ADDRESS_BUFFER_SIZE);
    }
    
    UI_InitColorScheme(&ui_context->color_scheme);
    ui_context->button_shadow_offset = vec2(5, 5);
}

// TODO(fakhri): support mouse
internal void 
UI_BeginFrame(UI_Context *ui_context)
{
    Assert(ui_context);
    ui_context->buffer_index = 0;
    ui_context->items_count = 0;
}

internal void
MenuItemLabel(Game_State *game_state, s8 text, v2 text_pos, b32 should_animate_color = 0)
{
    UI_Context        *ui_context        = &game_state->ui_context;
    Rendering_Context *rendering_context = &game_state->rendering_context;
    UI_ColorScheme    *color_scheme      = &ui_context->color_scheme;
    
    v3 color;
    if (should_animate_color)
    {
        f32 t = os->time - ui_context->last_time;
        f32 change = square_f(cos_f(PI * t));
        color = change * color_scheme->label_color1 + (1 - change) * color_scheme->label_color2;
    }
    else
    {
        color = color_scheme->label_color;
    }
    DebugDrawText(rendering_context, text, text_pos, color);
}

internal b32
MenuItemButton(Game_State *game_state, s8 item_text, v2 item_pos)
{
    UI_Context        *ui_context        = &game_state->ui_context;
    Rendering_Context *rendering_context = &game_state->rendering_context;
    Controller        *controller        = &os->controller;
    UI_ColorScheme    *color_scheme      = &ui_context->color_scheme;
    
    b32 clicked = 0;
    
    v3 color;
    b32 is_selected = (ui_context->selected_item == ui_context->items_count);
    if (is_selected)
    {
        if (controller->confirm.pressed)
        {
            controller->confirm.pressed = 0;
            clicked = 1;
        }
        // NOTE(fakhri): calculate color
        f32 t = os->time - ui_context->last_time;
        f32 change = square_f(cos_f(PI * t));
        color = change * color_scheme->button_text_active_color1 + (1 - change) * color_scheme->button_text_active_color2;
        
        DebugDrawText(rendering_context, item_text, item_pos + ui_context->button_shadow_offset, color_scheme->button_text_shadow_color);
    }
    else
    {
        color = color_scheme->button_text_color;
    }
    
    DebugDrawText(rendering_context, item_text, item_pos, color);
    
    ++ui_context->items_count;
    return clicked;
}

internal void
HandleInputFieldKeyboardInput(Buffer *input_buffer)
{
    // TODO(fakhri): support ctr+v
    // TODO(fakhri): support moving cursor with arrow keys
    OS_Event *event = 0;
    while(OS_GetNextEvent(&event))
    {
        if (event->type == OS_EventType_CharacterInput)
        {
            char c = (char)event->character;
            if (!IsBufferFull(input_buffer) && 
                (c == '.' || CharIsDigit(c)))
            {
                InsertCharacterToBuffer(input_buffer, c);
            }
            OS_EatEvent(event);
        }
        else if (event->type == OS_EventType_KeyPress && event->key == Key_Backspace && 
                 !IsBufferEmpty(input_buffer))
        {
            --(input_buffer->buffer.size);
        }
    }
}

internal void
MenuItemInputField(Game_State *game_state,v2 item_pos, v2 item_size)
{
    UI_Context        *ui_context        = &game_state->ui_context;
    Rendering_Context *rendering_context = &game_state->rendering_context;
    UI_ColorScheme    *color_scheme      = &ui_context->color_scheme;
    Controller        *controller        = &os->controller;
    
    Assert(ui_context->buffer_index < ArrayCount(ui_context->input_field_buffers));
    Buffer *input_buffer = ui_context->input_field_buffers + ui_context->buffer_index;
    
    v3 background_color;
    v3 text_color;
    
    
    b32 is_selected = (ui_context->selected_item == ui_context->items_count);
    if (is_selected)
    {
        HandleInputFieldKeyboardInput(input_buffer);
        
        // NOTE(fakhri): set colors
        background_color = color_scheme->input_field_background_active_color;
        text_color = color_scheme->input_field_text_active_color;
    }
    else
    {
        // NOTE(fakhri): set colors
        background_color = color_scheme->input_field_background_color;
        text_color = color_scheme->input_field_text_color;
    }
    
    // NOTE(fakhri): render input field background
    DebugDrawQuadScreenCoord(rendering_context, item_pos, item_size, background_color);
    
    DebugDrawText(rendering_context, input_buffer->buffer, item_pos, text_color);
    
    if (is_selected)
    {
        f32 t = os->time - ui_context->last_time;
        f32 change = square_f(cos_f(PI * t));
        v3 cursor_color = change * background_color + (1 - change) * text_color;
        
        v2 cursor_pos = vec2(item_pos.x +  0.5f * GetActiveFontWidth(rendering_context, input_buffer->buffer),
                             item_pos.y);
        
        v2 cursor_size = vec2(2, 1.1f * GetActiveFontHeight(rendering_context));
        DebugDrawQuadScreenCoord(rendering_context, cursor_pos, cursor_size, cursor_color);
    }
    
    ++ui_context->items_count;
    ++ui_context->buffer_index;
}
