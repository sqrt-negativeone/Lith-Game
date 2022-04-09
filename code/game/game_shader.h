/* date = March 29th 2022 9:55 pm */

#ifndef GAME_SHADER_H
#define GAME_SHADER_H

typedef GLuint Shader_ID;
typedef GLuint Vertex_Array_ID;
typedef GLuint Vertex_Buffer_ID;

enum Shader_Kind
{
    ShaderKind_None,
    ShaderKind_Quad,
    ShaderKind_Texture,
    ShaderKind_Font,
    
    ShaderKind_Count
};

struct Compile_Shader_Result
{
    b32 is_valid;
    Shader_ID program_id;
};

struct Shader_Hash_Slot
{
    String8 shader_name;
    Shader_Kind kind;
    
    Shader_Hash_Slot *next_in_hash;
};

struct Shaders_Hash
{
    Shader_Hash_Slot *shader_slots[131];
    u32 shaders_count;
};

struct Shader_Program
{
    b32 previously_loaded;
    
    Shader_ID id;
    Shader_ID vao;
    Shader_ID vbo;
};

#endif //GAME_SHADER_H
