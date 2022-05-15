
internal void
UI_ChangeSelectedItem(UI_Context *ui_context, i32 delta)
{
    Assert(ui_context);
    ui_context->last_select_time = os->time.game_time;
    ui_context->pressed_item = (ui_context->pressed_item + delta + ui_context->items_count) % ui_context->items_count;
}

internal void
UI_InitColorScheme(UI_ColorScheme *color_scheme)
{
    // TODO(fakhri): choose some good colors
    // TODO(fakhri): maybe load this from a config file?
    
    *color_scheme = {};
    
    v3 white = Vec3(1.0f, 1.0f, 1.0f);
    v3 none_white = 0.3f * white;
    v3 black = 0.f * white;
    v3 red = Vec3(1.0f, 0.f, 0.f);
    v3 blue = Vec3(78, 166, 206) / 255.f;
    
    color_scheme->label_color = white;
    color_scheme->label_color1 = white;
    color_scheme->label_color2 = 0.8f * white;
    
    // NOTE(fakhri): button colors
    color_scheme->button_text_color = 0.3f * white;
    color_scheme->button_text_active_color1 = 0.85f * blue;
    color_scheme->button_text_active_color2 = blue;
    color_scheme->button_text_shadow_color = black;
    color_scheme->button_text_clicked_color = red;
    
    // NOTE(fakhri): input field color
    color_scheme->input_field_background_color = 0.2f * white;
    color_scheme->input_field_background_active_color = 0.5f * white;
    
    color_scheme->input_field_text_color = color_scheme->input_field_background_active_color;
    color_scheme->input_field_text_active_color = color_scheme->input_field_background_color;
}

internal void
UI_Init(UI_Context *ui_context, Render_Context *render_context)
{
    ui_context->render_context = render_context;
    UI_InitColorScheme(&ui_context->color_scheme);
}

internal void 
UI_BeginFrame(UI_Context *ui_context, Controller *controller, Render_Context *render_context)
{
    Assert(ui_context);
    ui_context->items_count = 0;
    ui_context->controller = controller;
    ui_context->render_context = render_context;
}

internal void
UI_EndFrame(UI_Context *ui_context, Controller *controller)
{
    // NOTE(fakhri): process input
    if (controller->move_down.pressed)
    {
        UI_ChangeSelectedItem(ui_context, +1);
    }
    if (controller->move_up.pressed)
    {
        UI_ChangeSelectedItem(ui_context, -1);
    }
}

internal void
UI_Label(UI_Context *ui_context, String8 text, f32 x, f32 y, Font_Kind font_to_use, b32 should_animate_color = false)
{
    UI_ColorScheme *color_scheme = &ui_context->color_scheme;
    
    f32 change = should_animate_color? Square(CosF(PI32 * os->time.game_time)) : 0;
    v3 color = Lerp(color_scheme->label_color1, change, color_scheme->label_color2);
    
    Render_PushText(ui_context->render_context, text, Vec3(x, y, 0), Vec4(color, 1), CoordinateType_World, font_to_use);
}

internal b32
UI_Button(UI_Context *ui_context, String8 item_text, f32 x, f32 y, Font_Kind font_to_use)
{
    UI_ColorScheme    *color_scheme = &ui_context->color_scheme;
    
    b32 clicked = 0;
    
    v2 item_pos = Vec2(x, y);
    
    v3 text_color;
    
    v2 hitbox = 1.2f * Vec2(GetFontWidth(ui_context->render_context, font_to_use, item_text), 
                            GetFontHeight(ui_context->render_context, font_to_use));
    
    hitbox = SizeFromScreenCoordsToWorldCoords(ui_context->render_context, hitbox);
    if (IsInsideRect(RectCentDim(item_pos, hitbox), WorldCoordsFromScreenCoords(ui_context->render_context, os->mouse_position).xy))
    {
        ui_context->last_select_time = os->time.game_time;
        ui_context->pressed_item = ui_context->items_count;
    }
    
    b32 is_selected = (ui_context->pressed_item == ui_context->items_count);
    
    if (is_selected)
    {
        if (ui_context->controller->confirm.pressed || ui_context->controller->left_mouse.pressed)
        {
            clicked = 1;
        }
        // TODO(fakhri): maybe show some change in color when the button is clicked?
        f32 t = os->time.game_time - ui_context->last_select_time;
        f32 change = Square(CosF(PI32 * t));
        text_color = Lerp(color_scheme->button_text_active_color1, change, color_scheme->button_text_active_color2);
        v2 shadow_offset = Vec2(MiliMeter(2), -MiliMeter(1));
        Render_PushText(ui_context->render_context, item_text, Vec3(item_pos + shadow_offset, 0), Vec4(text_color, 1), CoordinateType_World, font_to_use);
        
    }
    else
    {
        text_color = color_scheme->button_text_color;
    }
    
    Render_PushText(ui_context->render_context, item_text, Vec3(item_pos, 0), Vec4(text_color, 1), CoordinateType_World, font_to_use);
    
    ++ui_context->items_count;
    return clicked;
}

internal void
HandleInputFieldKeyboardInput(Buffer *input_buffer)
{
    // TODO(fakhri): support ctr+v
    // TODO(fakhri): support moving cursor with arrow keys
    
    OS_Event *event = os->events.first;
    while(event)
    {
        switch(event->kind)
        {
            case OS_EventKind_Text:
            {
                char c = (char)event->character;
                if (!IsBufferFull(input_buffer))
                {
                    InsertCharacterToBuffer(input_buffer, c);
                }
            } break;
            case OS_EventKind_Press:
            {
                if (event->key == OS_Key_Backspace && 
                    !IsBufferEmpty(input_buffer))
                {
                    if (event->modifiers & OS_Modifier_Ctrl)
                    {
                        EmptyBuffer(input_buffer);
                    }
                    else
                    {
                        RemoveLastCharacterFromBuffer(input_buffer);
                    }
                }
                else if (event->key == OS_Key_V && (event->modifiers & OS_Modifier_Ctrl))
                {
                    // TODO(fakhri): insert the content of the clipboad into the buffer
                    Log("Pasting clipboad content to the buffer");
                }
            } break;
        }
        event = event->next;
    }
}

internal void
UI_InputField(UI_Context *ui_context, v2 item_size, f32 x, f32 y, Buffer *input_buffer, Font_Kind font_to_use)
{
    UI_ColorScheme    *color_scheme      = &ui_context->color_scheme;
    
    v3 background_color;
    v3 text_color;
    
    v2 item_pos = Vec2(x, y);
    
    if (IsInsideRect(RectCentDim(item_pos, item_size), WorldCoordsFromScreenCoords(ui_context->render_context, os->mouse_position).xy))
    {
        ui_context->pressed_item = ui_context->items_count;
    }
    
    b32 is_selected = (ui_context->pressed_item == ui_context->items_count);
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
    Render_PushQuad(ui_context->render_context, Vec3(item_pos, 0), item_size, Vec4(background_color, 1), CoordinateType_World);
    
    Render_PushText(ui_context->render_context, input_buffer->content, Vec3(item_pos, 0), Vec4(text_color, 1), CoordinateType_World, font_to_use);
    
    if (is_selected)
    {
        f32 t = os->time.game_time - ui_context->last_select_time;
        f32 change = Square(CosF(PI32 * t));
        v3 cursor_color = Lerp(background_color, change, text_color);
        
        f32 meter_per_pixel = 1.0f / ui_context->render_context->pixels_per_meter;
        f32 font_half_width_in_pixels = 0.5f * GetFontWidth(ui_context->render_context, font_to_use, input_buffer->content);
        v2 cursor_size = Vec2(MiliMeter(2), CentiMeter(4));
        v2 cursor_pos = Vec2(item_pos.x + meter_per_pixel * font_half_width_in_pixels, item_pos.y);
        Render_PushQuad(ui_context->render_context, Vec3(cursor_pos, 0), cursor_size, Vec4(cursor_color, 1), CoordinateType_World);
    }
    
    ++ui_context->items_count;
}

