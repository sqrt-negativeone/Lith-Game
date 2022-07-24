
internal Game_UI
UI_Init(Render_Context *render_context, Controller *controller)
{
    Game_UI ui = {};
    ui.render_context = render_context;
    ui.controller = controller;
    return ui;
}

internal void
UI_Begin(Game_UI *ui)
{
    ui->current_widget = 0;
}

internal void
UI_End(Game_UI *ui)
{
    
}

internal void
ChangeActiveFont(Game_UI *ui, Font_Kind new_font)
{
    ui->active_font = new_font;
}

internal void
ChangeActiveCoordinates(Game_UI *ui, Coordinate_Type new_coordinates)
{
    ui->active_coordinates = new_coordinates;
}

internal f32
VerticalAdvanceFontHeight(Game_UI *ui)
{
    f32 result = 1.4f * GetFontHeight(ui->render_context, ui->active_font);
    return result;
}

internal u32
GetCurrentWidgetID(Game_UI *ui)
{
    u32 result = ++ui->current_widget;
    return result;
}

internal void
UI_Label(Game_UI *ui, f32 x, f32 y, String8 label_text, v4 text_color, f32 fade_in = 1.0f)
{
    v3 label_position = Vec3(x, y, Meter(1));
    text_color.a *= fade_in;
    Render_PushText(ui->render_context, label_text, label_position, text_color, ui->active_coordinates, ui->active_font);
}

internal b32
UI_Button(Game_UI *ui, f32 x, f32 y, String8 button_text, f32 dt, f32 fade_in = 1.0f, b32 accept_input = true)
{
    v4 hover_color = Vec4(0.2f,0.5f,0.5f,1.0f);
    v4 text_color  = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    u32 widget_id  = GetCurrentWidgetID(ui);
    
    b32 is_pressed = 0;
    f32 horizontal_padding = 0.1f;
    f32 vertical_padding   = 0.1f;
    f32 width  = (1.0f + horizontal_padding) * 
        GetFontWidth(ui->render_context, ui->active_font, button_text);
    f32 height = (1.0f + vertical_padding) * 
        GetFontHeight(ui->render_context, ui->active_font);
    
    v3 text_position = Vec3(x, y, Meter(1));
    v3 button_position = Vec3(x, y - 0.25f * height, Meter(1));
    v2 size = Vec2(width, height);
    v2 size_in_meters = SizeFromScreenCoordsToWorldCoords(ui->render_context, size); 
    
    if (accept_input)
    {
        if (IsInsideRect(RectCentDim(button_position.xy, size),
                         os->mouse_position))
        {
            if (ui->active_widget != widget_id)
            {
                ui->active_widget = widget_id;
                ui->active_transition = 0.0f;
                ui->active_transition_speed = 9.0f;
            }
            
            ui->active_transition += ui->active_transition_speed * dt;
            ui->active_transition = ClampTop(ui->active_transition, 1.0f);
            v4 background_color = hover_color;
            v2 background_size_in_meters = size_in_meters;
            
            background_color.a *= ui->active_transition;
            background_size_in_meters.height *= ui->active_transition;
            
            if (ui->controller->left_mouse.pressed)
            {
                ui->hot_widget = widget_id;
                ui->hot_transition = 0.0f;
                ui->hot_transition_speed = 20.0f;
            }
            
            if (ui->hot_widget == widget_id)
            {
                ui->hot_transition += ui->hot_transition_speed * dt;
                if (ui->controller->left_mouse.released)
                {
                    ui->hot_transition_speed *= -3;
                }
                
                if (ui->hot_transition > 1.0f)
                {
                    ui->hot_transition = 1.0f;
                }
                
                
                if (ui->hot_transition < 0)
                {
                    ui->hot_widget = 0;
                    ui->hot_transition = 0;
                    is_pressed = 1;
                }
                
                v3 click_color = Vec3(1, 1, 1) - background_color.rgb;
                f32 t = ui->hot_transition;
                background_color.rgb = (1 - t) * background_color.rgb + t * click_color;
                background_size_in_meters.height *= (1.0f - 0.1f * ui->hot_transition);
            }
            
            background_color.a *= fade_in;
            Render_PushQuad(ui->render_context, button_position, background_size_in_meters, background_color, ui->active_coordinates);
        }
        else
        {
            if (ui->active_widget == widget_id)
            {
                ui->active_widget = 0;
            }
            
            if (ui->hot_widget == widget_id)
            {
                ui->hot_widget = 0;
            }
        }
    }
    
    text_color.a *= fade_in;
    Render_PushText(ui->render_context, button_text,  text_position, text_color, ui->active_coordinates, ui->active_font);
    
    return is_pressed;
}


internal b32
InputField_AddCharacter(Game_UI *ui, Game_UI_InputField *input_field, u8 character)
{
    b32 result = false;
    if (input_field->size < ArrayCount(input_field->buffer))
    {
        result = true;
        for (u32 index = input_field->size;
             index > input_field->cursor_index;
             --index)
        {
            input_field->buffer[index] = input_field->buffer[index - 1];
        }
        input_field->buffer[input_field->cursor_index] = character;
        ++input_field->size;
        ++input_field->cursor_index;
        input_field->cursor_target_x_offset += 0.5f * GetCharacterAdvance(ui->render_context, ui->active_font, character);
    }
    return result;
}

internal void
InputField_DeleteCharacterAtCursor(Game_UI *ui, Game_UI_InputField *input_field)
{
    if (input_field->size > 0 && input_field->cursor_index > 0)
    {
        u8 character = input_field->buffer[input_field->cursor_index - 1];
        for (u32 index = input_field->cursor_index - 1;
             index < input_field->size - 1;
             ++index)
        {
            input_field->buffer[index] = input_field->buffer[index + 1];
        }
        --input_field->size;
        --input_field->cursor_index;
        input_field->cursor_target_x_offset -= 0.5f * GetCharacterAdvance(ui->render_context, ui->active_font, character);
    }
}

internal void
InputField_MoveLeft(Game_UI *ui, Game_UI_InputField *input_field)
{
    if (input_field->cursor_index > 0)
    {
        u8 character = input_field->buffer[input_field->cursor_index - 1];
        --input_field->cursor_index;
        input_field->cursor_target_x_offset -= GetCharacterAdvance(ui->render_context, ui->active_font, character);
    }
}

internal void
InputField_MoveRight(Game_UI *ui, Game_UI_InputField *input_field)
{
    if (input_field->cursor_index < input_field->size)
    {
        u8 character = input_field->buffer[input_field->cursor_index];
        ++input_field->cursor_index;
        input_field->cursor_target_x_offset += GetCharacterAdvance(ui->render_context, ui->active_font, character);
    }
}

internal void
UI_InputField(Game_UI *ui, Game_UI_InputFieldKind input_field_kind, f32 x, f32 y, f32 dt, f32 fade_in = 1.0f, b32 accept_input = true)
{
    Game_UI_InputField *input_field = ui->input_fields + input_field_kind;
    b32 is_pressed = 0;
    v4 background_color = Vec4(0.3f, 0.3f, 0.3f, fade_in);
    v4 hover_color = Vec4(0.2f, 0.5f, 0.5f, fade_in);
    v4 text_color  = Vec4(1.0f, 1.0f, 1.0f, fade_in);
    
    u32 widget_id  = GetCurrentWidgetID(ui);
    
    f32 horizontal_padding = 0.1f;
    f32 vertical_padding   = 0.1f;
    
    f32 width  = (1.0f + horizontal_padding) * 
        GetFontMaxWidth(ui->render_context, ui->active_font, ArrayCount(input_field->buffer));
    f32 height = (1.0f + vertical_padding) * 
        GetFontHeight(ui->render_context, ui->active_font);
    
    v3 text_position = Vec3(x, y, Meter(1));
    v3 input_field_position = Vec3(x, y - 0.25f * height, Meter(1));
    v2 size = Vec2(width, height);
    v2 size_in_meters = SizeFromScreenCoordsToWorldCoords(ui->render_context, size); 
    
    if (accept_input)
    {
        if (IsInsideRect(RectCentDim(input_field_position.xy, size),
                         os->mouse_position))
        {
            if (ui->active_widget != widget_id)
            {
                ui->active_widget = widget_id;
                ui->active_transition = 0.0f;
                ui->active_transition_speed = 5.0f;
            }
            
            ui->active_transition += ui->active_transition_speed * dt;
            if (ui->active_transition > 1.0f)
            {
                ui->active_transition = 1.0f;
            }
            ui->active_transition = ClampTop(ui->active_transition, 1.0f);
            
            
            if (ui->controller->left_mouse.pressed)
            {
                ui->hot_widget = widget_id;
                ui->hot_transition = 0.0f;
                ui->hot_transition_speed = 10.0f;
                is_pressed = 1;
            }
            
            if (ui->hot_widget == widget_id)
            {
                ui->hot_transition += ui->hot_transition_speed * dt;
                
                if (ui->hot_transition > 1.0f)
                {
                    ui->hot_transition = 1.0f;
                    ui->hot_transition_speed *= -1;
                }
                
                if (ui->hot_transition < 0)
                {
                    ui->hot_widget = 0;
                    ui->hot_transition = 0;
                }
            }
        }
        else
        {
            if (ui->hot_widget == widget_id)
            {
                ui->hot_widget = 0;
                ui->hot_transition = 0;
            }
        }
    }
    
    if (ui->active_widget == widget_id)
    {
        background_color = ((1.0f - ui->active_transition) * background_color + 
                            ui->active_transition * Vec4(0.6f, 0.6f, 0.6f, fade_in));
        
        // NOTE(fakhri): handle input events
        {
            OS_Event *event = os->events.first;
            while(event)
            {
                switch(event->kind)
                {
                    case OS_EventKind_Text:
                    {
                        InputField_AddCharacter(ui, input_field, (u8)event->character);
                    } break;
                    case OS_EventKind_Press:
                    {
                        if (event->key == OS_Key_Backspace)
                        {
                            InputField_DeleteCharacterAtCursor(ui, input_field);
                        }
                        else if (event->key == OS_Key_Left)
                        {
                            InputField_MoveLeft(ui, input_field);
                        }
                        else if (event->key == OS_Key_Right)
                        {
                            InputField_MoveRight(ui, input_field);
                        }
                        else if (event->key == OS_Key_V && (event->modifiers & OS_Modifier_Ctrl))
                        {
                            M_Temp scratch = GetScratch(0 ,0);
                            String clipboard_data = os->GetStringFromClipboard(scratch.arena);
                            for (u32 index = 0;
                                 index < clipboard_data.size;
                                 ++index)
                            {
                                if (!InputField_AddCharacter(ui, input_field, clipboard_data.str[index]))
                                {
                                    break;
                                }
                            }
                            ReleaseScratch(scratch);
                        }
                    } break;
                    
                    default: break;
                }
                event = event->next;
            }
        }
        
        // NOTE(fakhri): hot effect
        if (ui->hot_widget == widget_id)
        {
            size_in_meters.height *= (1.0f - 0.05f * ui->hot_transition);
        }
        
        if (is_pressed)
        {
            
            Render_PushText(ui->render_context, Str8Lit("Pressed"), text_position, text_color, ui->active_coordinates, ui->active_font);
            
            // NOTE(fakhri): move the cursor to where it should be
            f32 mouse_offset = os->mouse_position.x - input_field_position.x;
            String8 text = Str8(input_field->buffer, input_field->size);
            f32 text_width = GetFontWidth(ui->render_context, ui->active_font, text);
            f32 offset = -0.5f * text_width;
            u32 index = 0;
            for (;
                 index < text.size;
                 ++index)
            {
                f32 prev_offset = offset;
                offset += GetCharacterAdvance(ui->render_context, ui->active_font, text.str[index]);
                if (offset > mouse_offset)
                {
                    // NOTE(fakhri): we go to the nearest offset from where we clicked
                    f32 half_offset = 0.5f * (offset + prev_offset);
                    if (mouse_offset > half_offset)
                    {
                        index = index + 1;
                    }
                    else
                    {
                        offset = prev_offset;
                    }
                    break;
                }
            }
            
            input_field->cursor_index = index;
            input_field->cursor_target_x_offset = offset;
        }
        
        // NOTE(fakhri): render the cursor
        {
            v2 cursor_size = Vec2(MiliMeter(2), PixelsToMeter(ui->render_context, 0.95f * height));
            
            f32 t = Square(SinF(4.0f * os->time.wall_time));
            v3 cursor_color1 = background_color.xyz;
            v3 cursor_color2 = 2 * Vec3(0.1f, 0.1f, 0.1f);
            v3 cursor_color = (1 - t) * cursor_color1 + t * cursor_color2;
            v3 cursor_position = input_field_position; 
            
            SpringMoveTowards(&input_field->cursor_x_offset, input_field->cursor_target_x_offset,
                              &input_field->cursor_x_offset_speed, 200, 10, 0.5f, dt);
            
            cursor_position.x += input_field->cursor_x_offset;
            cursor_position.z += MiliMeter(1);
            Render_PushQuad(ui->render_context, cursor_position, cursor_size, Vec4(cursor_color, fade_in), ui->active_coordinates);
        }
        
    }
    
    Render_PushQuad(ui->render_context, input_field_position, size_in_meters, background_color, ui->active_coordinates);
    
    Render_PushText(ui->render_context, Str8(input_field->buffer, input_field->size), text_position, text_color, ui->active_coordinates, ui->active_font);
    
}

internal void
UI_OpenMenu(Game_UI *ui, GameMenuKind menu_kind)
{
    if (ui->active_menu)
    {
        Game_Menu *menu = ui->menus + ui->active_menu;
        menu->accept_input = 0;
        menu->presence_change_speed = -10.0f;
    }
    
    ui->active_menu = menu_kind;
    {
        Game_Menu *menu = ui->menus + ui->active_menu;
        menu->accept_input = 1;
        menu->presence_change_speed = 10.0f;
    }
    
}

internal void
UI_CloseMenu(Game_UI *ui)
{
    UI_OpenMenu(ui, GameMenuKind_None);
}

internal b32
UI_MenuSection(Game_UI *ui, GameMenuKind menu_kind, f32 dt)
{
    ui->current_menu = menu_kind;
    Game_Menu *menu = ui->menus + menu_kind;
    menu->presence += menu->presence_change_speed * dt;
    menu->presence = Clamp(0.0f, menu->presence, 1.0f);
    
    b32 result = ((ui->active_menu == menu_kind) || (menu->presence > 0));
    return result;
}