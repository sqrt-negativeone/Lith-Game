
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
    ui->widgets_count = 0;
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

internal void
UI_Label(Game_UI *ui, f32 x, f32 y, String8 label_text, v4 text_color, f32 fade_in = 1.0f)
{
    v3 label_position = Vec3(x, y, Meter(1));
    text_color.a *= fade_in;
    Render_PushText(ui->render_context, label_text, label_position, text_color, ui->active_coordinates, ui->active_font);
}

internal u32
GetCurrentWidgetID(Game_UI *ui)
{
    u32 result = ++ui->current_widget;
    return result;
}

internal b32
UI_Button(Game_UI *ui, f32 x, f32 y, String8 button_text, v4 text_color, v4 hover_color, f32 dt, f32 fade_in = 1.0f, b32 accept_input = true)
{
    u32 widget_id = GetCurrentWidgetID(ui);
    
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
            if (ui->active_transition > 1.0f)
            {
                ui->active_transition = 1.0f;
            }
            
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