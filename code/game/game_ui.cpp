
internal void
UI_ChangeSelectedItem(UI_Context *ui_context, i32 delta)
{
    Assert(ui_context);
    ui_context->last_select_time = os->time.game_time;
    ui_context->selected_item = (ui_context->selected_item + delta + ui_context->items_count) % ui_context->items_count;
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
UI_Init(UI_Context *ui_context, M_Arena *arena)
{
    UI_InitColorScheme(&ui_context->color_scheme);
}

internal void 
UI_BeginFrame(UI_Context *ui_context)
{
    Assert(ui_context);
    ui_context->items_count = 0;
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
UI_Label(Game_State *game_state, String8 text, f32 x, f32 y, b32 should_animate_color = false)
{
    UI_Context        *ui_context        = &game_state->ui_context;
    UI_ColorScheme    *color_scheme      = &ui_context->color_scheme;
    
    f32 change = should_animate_color? Square(CosF(PI32 * os->time.game_time)) : 0;
    v3 color = Lerp(color_scheme->label_color1, change, color_scheme->label_color2);
    
    DrawTextScreenCoord(game_state, text, Vec2(x, y), color);
}

internal b32
UI_Button(Game_State *game_state, String8 item_text, f32 x, f32 y, v2 hitbox)
{
    UI_Context        *ui_context        = &game_state->ui_context;
    Controller        *controller        = &game_state->controller;
    UI_ColorScheme    *color_scheme      = &ui_context->color_scheme;
    
    b32 clicked = 0;
    
    v2 item_pos = Vec2(x, y);
    
    v3 text_color;
    
    //DrawQuadScreenCoord(game_state, Vec3(x, y, 0), hitbox, Vec3(1, 1, 1));
    if (IsInsideRect(RectCentDim(item_pos, hitbox), os->mouse_position ))
    {
        ui_context->last_select_time = os->time.game_time;
        ui_context->selected_item = ui_context->items_count;
    }
    
    b32 is_selected = (ui_context->selected_item == ui_context->items_count);
    
    if (is_selected)
    {
        if (controller->confirm.pressed || controller->left_mouse.pressed)
        {
            clicked = 1;
        }
        // TODO(fakhri): maybe show some change in color when the button is clicked?
        f32 t = os->time.game_time - ui_context->last_select_time;
        f32 change = Square(CosF(PI32 * t));
        text_color = Lerp(color_scheme->button_text_active_color1, change, color_scheme->button_text_active_color2);
        v2 shadow_offset = Vec2(5, 5);
        DrawTextScreenCoord(game_state, item_text, item_pos + shadow_offset, color_scheme->button_text_shadow_color);
    }
    else
    {
        text_color = color_scheme->button_text_color;
    }
    
    DrawTextScreenCoord(game_state, item_text, item_pos, text_color);
    
    ++ui_context->items_count;
    return clicked;
}

internal void
HandleInputFieldKeyboardInput(Buffer *input_buffer)
{
    // TODO(fakhri): support ctr+v
    // TODO(fakhri): support moving cursor with arrow keys
    NotImplemented;
#if 0
    OS_Event *event = 0;
    while(OS_GetNextEvent(&event))
    {
        if (event->type == OS_EventType_CharacterInput)
        {
            char c = (char)event->character;
            if (!IsBufferFull(input_buffer))
            {
                InsertCharacterToBuffer(input_buffer, c);
            }
            OS_EatEvent(event);
        }
        else if (event->type == OS_EventType_KeyPress)
        {
            if (event->key == Key_Backspace && 
                !IsBufferEmpty(input_buffer))
            {
                if (event->modifiers & KeyModifier_Ctrl)
                {
                    EmptyBuffer(input_buffer);
                }
                else
                {
                    RemoveLastCharacterFromBuffer(input_buffer);
                }
            }
        }
        else if (event->key == Key_V && (event->modifiers & KeyModifier_Ctrl))
        {
            // TODO(fakhri): insert the content of the clipboad into the buffer
            Log("Pasting clipboad content to the buffer");
        }
    }
#endif
}

internal void
UI_InputField(Game_State *game_state, v2 item_size, f32 x, f32 y, Buffer *input_buffer)
{
    UI_Context        *ui_context        = &game_state->ui_context;
    UI_ColorScheme    *color_scheme      = &ui_context->color_scheme;
    Controller        *controller        = &game_state->controller;
    
    v3 background_color;
    v3 text_color;
    
    v2 item_pos = Vec2(x, y);
    
    if (IsInsideRect(RectCentDim(item_pos, item_size), os->mouse_position ))
    {
        ui_context->selected_item = ui_context->items_count;
    }
    
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
    DrawQuadScreenCoord(game_state, Vec3(item_pos, 0), item_size, background_color);
    
    DrawTextScreenCoord(game_state, input_buffer->content, item_pos, text_color);
    
    if (is_selected)
    {
        f32 t = os->time.game_time - ui_context->last_select_time;
        f32 change = Square(CosF(PI32 * t));
        v3 cursor_color = Lerp(background_color, change, text_color);
        
        v2 cursor_pos = Vec2(item_pos.x +  0.5f * GetFontWidth(game_state->active_font, input_buffer->content),
                             item_pos.y);
        
        v2 cursor_size = Vec2(2, 1.1f * GetFontHeight(game_state->active_font));
        DrawQuadScreenCoord(game_state, Vec3(cursor_pos, 0), cursor_size, cursor_color);
    }
    
    ++ui_context->items_count;
}
