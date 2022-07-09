
internal Compile_Shader_Result 
CompileShader(String8 shader_name)
{
    Compile_Shader_Result result = {};
    String8 shader_path = {};
    
    M_Temp scratch = GetScratch(0, 0);
    {
        String8List path_list = {};
        Str8ListPush(scratch.arena, &path_list, Str8Lit("data/shaders/"));
        Str8ListPush(scratch.arena, &path_list, shader_name);
        shader_path = Str8ListJoin(scratch.arena, path_list, 0);
    }
    
    String8 shader_file = os->LoadEntireFile(scratch.arena, shader_path);
    if (shader_file.size == 0)
    {
        LogError("Couldn't open the shader file %s", shader_path.cstr);
        goto end;
    }
    
    const char *vertex_shader_source[3] = {"#version 460 core\n","#define VERTEX_SHADER\n", shader_file.cstr};
    const char *fragment_shader_source[3] = {"#version 460 core\n", "#define FRAGMENT_SHADER\n", shader_file.cstr};
    
    Shader_ID vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    Shader_ID fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    
    // NOTE(fakhri): compile vertex shader
    {
        glShaderSource(vertex_shader_id, 3, vertex_shader_source, 0);
        glCompileShader(vertex_shader_id);
        GLint success = 0;
        glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &success);
        if (success != GL_TRUE)
        {
            char infoLog[512];
            glGetShaderInfoLog(vertex_shader_id, sizeof(infoLog), 0, infoLog);
            LogError("Couldn't compile vertex shader program");
            LogError("shader error :\n%s", infoLog);
            glDeleteShader(vertex_shader_id);
            goto end;
        }
    }
    
    // NOTE(fakhri): compiler fragment shader
    {
        glShaderSource(fragment_shader_id, 3, fragment_shader_source, 0);
        glCompileShader(fragment_shader_id);
        GLint success = 0;
        glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &success);
        if (success != GL_TRUE)
        {
            char infoLog[512];
            glGetShaderInfoLog(fragment_shader_id, sizeof(infoLog), 0, infoLog);
            LogError("Couldn't compile fragment shader program");
            LogError("shader error :\n%s", infoLog);
            glDeleteShader(fragment_shader_id);
            goto end;
        }
    }
    
    // NOTE(fakhri): link shader program
    {
        Shader_ID program = glCreateProgram();
        glAttachShader(program, vertex_shader_id);
        glAttachShader(program, fragment_shader_id);
        glLinkProgram(program);
        GLint success = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (success == GL_TRUE)
        {
            result.is_valid = true;
            glDetachShader(program, vertex_shader_id);
            glDetachShader(program, fragment_shader_id);
            result.program_id = program;
        }
        else
        {
            char infoLog[512];
            glGetProgramInfoLog(result.program_id, sizeof(infoLog), 0, infoLog);
            LogError("Couldn't link shader program\n");
            LogError("shader error :\n%s", infoLog);
            glDeleteProgram(result.program_id);
            goto end;
        }
    }
    
    if (result.is_valid)
    {
        Log("Shader %s compiled successfully", shader_path.str); 
    }
    
    end:;
    ReleaseScratch(scratch);
    return result;
}

internal void
SetupShader(Render_Context *render_context, Shader_Kind kind)
{
    local_persist f32 quad_vertices[] = {
        // position
        -0.5f, -0.5f,
        -0.5f, +0.5f,
        +0.5f, +0.5f,
        
        -0.5f, -0.5f,
        +0.5f, +0.5f,
        +0.5f, -0.5f,
    };
    
    f32 min_x = 0.0f, min_y = 0.0f;
    f32 max_x = 1.0f, max_y = 1.0f;
    
    local_persist f32 texture_vertices[] = {
        // position         texture
        -0.5f, -0.5f, 	 0.0f, 0.0f,
        -0.5f, +0.5f, 	 0.0f, 1.0f,
        +0.5f, +0.5f, 	 1.0f, 1.0f,
        
        -0.5f, -0.5f, 	 0.0f, 0.0f,
        +0.5f, +0.5f, 	 1.0f, 1.0f,
        +0.5f, -0.5f, 	 1.0f, 0.0f,
    };
    
    Shader_Program *program = render_context->shaders + kind;
    glUseProgram(program->id);
    
    // NOTE(fakhri): per shader kind init
    if (!render_context->shaders[kind].prevly_loaded)
    {
        // NOTE(fakhri): stuff that only need to happen the first time we load the shader
        program->prevly_loaded = true;
        
        // NOTE(fakhri): shared settings
        glGenVertexArrays(1, &program->vao);
        glBindVertexArray(program->vao);
        
        glGenBuffers(1, &program->vbo);
        glBindBuffer(GL_ARRAY_BUFFER, program->vbo);
        
        switch(kind)
        {
            case ShaderKind_None: break;
            case ShaderKind_Quad:
            {
                glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
                
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), (void*)0);
                
            } break;
            case ShaderKind_Texture:
            {
                glBufferData(GL_ARRAY_BUFFER, sizeof(texture_vertices), texture_vertices, GL_STATIC_DRAW);
                
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void *)0);
            } break;
            default:
            {
                Assert(kind < ShaderKind_Count);
                NotImplemented;
            }break;
        }
    }
    
    glUseProgram(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

internal u32
ComputeHashShaderPath(String8 shader_path)
{
    u32 hash  = 0;
    for (u32 index = 0;
         index < shader_path.size;
         ++index)
    {
        u8 ch = shader_path.str[index];
        hash += (hash * 33) ^ ch;
    }
    return hash;
}


read_only global char *shaders_names[ShaderKind_Count] = {
    "quad_shader.glsl",
    "quad_shader.glsl",
    "texture_shader.glsl",
};

internal void
LoadShader(Render_Context *render_context, Shader_Kind shader_kind)
{
    if (shader_kind == ShaderKind_None) return;
    
    String8 shader_name = Str8C(shaders_names[shader_kind]);
    
    // NOTE(fakhri): find the shader hash slot
    u32 hash_index  = ComputeHashShaderPath(shader_name);
    hash_index %= ArrayCount(render_context->shaders_hash.shader_slots);
    
    // NOTE(fakhri): create new slot and add the hash table
    Shader_Hash_Slot *new_shader_slot = PushStruct(os->permanent_arena, Shader_Hash_Slot);
    Assert(new_shader_slot);
    new_shader_slot->shader_name = PushStr8Copy(os->permanent_arena, shader_name);
    new_shader_slot->kind = shader_kind;
    new_shader_slot->next_in_hash = render_context->shaders_hash.shader_slots[hash_index];
    render_context->shaders_hash.shader_slots[hash_index] = new_shader_slot;
    
    // NOTE(fakhri): compile the shader
    Compile_Shader_Result shader_result = CompileShader(shader_name);
    if (shader_result.is_valid)
    {
        render_context->shaders[shader_kind].id = shader_result.program_id;
        SetupShader(render_context, shader_kind);
    }
    else
    {
        InvalidPath;
    }
    
}

exported
APP_HotLoadShader(HotLoadShader)
{
    // NOTE(fakhri): compute hash
    u32 hash_index  = ComputeHashShaderPath(shader_name);
    hash_index %= ArrayCount(game_state->render_context.shaders_hash.shader_slots);
    
    b32 found = false;
    for (Shader_Hash_Slot *shader_hash_slot = game_state->render_context.shaders_hash.shader_slots[hash_index];
         shader_hash_slot;
         shader_hash_slot = shader_hash_slot->next_in_hash)
    {
        if (Str8Match(shader_name, shader_hash_slot->shader_name, MatchFlag_CaseInsensitive))
        {
            found = true;
            // NOTE(fakhri): recompile the shader
            Compile_Shader_Result compile_result = CompileShader(shader_name);
            Assert(shader_hash_slot->kind < ArrayCount(game_state->render_context.shaders));
            Shader_Kind shader_kind = shader_hash_slot->kind;
            if (compile_result.is_valid)
            {
                glDeleteProgram(game_state->render_context.shaders[shader_kind].id);
                game_state->render_context.shaders[shader_kind].id = compile_result.program_id;
                SetupShader(&game_state->render_context, shader_kind);
            }
            else
            {
                LogError("Couldn't Recompile Shader %s", shader_name.cstr);
            }
        }
    }
    
    if (!found)
    {
        Log("Make sure that you added %s to the hashtable, couldn't find it", shader_name.cstr);
    }
}
