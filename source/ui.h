/* date = February 2nd 2022 1:28 am */

#ifndef UI_H
#define UI_H

struct UI_ColorScheme
{
    // NOTE(fakhri): text labels color
    v3 label_color;
    v3 label_color1;
    v3 label_color2;
    
    // NOTE(fakhri): button colors
    v3 button_text_color;
    v3 button_text_active_color1;
    v3 button_text_active_color2;
    v3 button_text_shadow_color;
    
    // NOTE(fakhri): input field color
    v3 input_field_background_color;
    v3 input_field_text_color;
    
    v3 input_field_background_active_color;
    v3 input_field_text_active_color;
};

#define MAX_INPUT_FIELDS_PER_MENU 1
#define SERVER_ADDRESS_BUFFER_SIZE 16

struct UI_Context
{
    u32 selected_item;
    u32 items_count;
    v2 button_shadow_offset;
    
    u32 buffer_index;
    Buffer input_field_buffers[MAX_INPUT_FIELDS_PER_MENU];
    
    f32 last_time;
    
    UI_ColorScheme color_scheme;
};

#endif //UI_H
