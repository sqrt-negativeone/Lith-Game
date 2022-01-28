
struct Compile_Shader_Result
{
    b32 is_valid;
    GLuint program;
};

internal
Compile_Shader_Result CompileShader(M_Arena *arena, s8 shader_path)
{
    Compile_Shader_Result result = {};
    
    s8 shader_file = {};
    os->LoadEntireFile(arena,shader_path, &shader_file.data, &shader_file.size);
    
    
    const char *vertex_shader_source[3] = {"#version 460 core\n","#define VERTEX_SHADER\n", shader_file.str};
    const char *fragment_shader_source[3] = {"#version 460 core\n", "#define FRAGMENT_SHADER\n", shader_file.str};
    
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    
    // NOTE(fakhri): compile vertex shader
    glShaderSource(vertex_shader_id, 3, vertex_shader_source, 0);
    glCompileShader(vertex_shader_id);
    GLint success = 0;
    glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &success);
    if (success == GL_TRUE)
    {
        // NOTE(fakhri): compiler fragment shader
        glShaderSource(fragment_shader_id, 3, fragment_shader_source, 0);
        glCompileShader(fragment_shader_id);
        glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &success);
        if (success == GL_TRUE)
        {
            // NOTE(fakhri): link shader program
            GLuint program = glCreateProgram();
            glAttachShader(program, vertex_shader_id);
            glAttachShader(program, fragment_shader_id);
            glLinkProgram(program);
            glGetProgramiv(program, GL_LINK_STATUS, &success);
            if (success == GL_TRUE)
            {
                result.is_valid = true;
                glDetachShader(program, vertex_shader_id);
                glDetachShader(program, fragment_shader_id);
                result.program = program;
            }
            else
            {
                char infoLog[512];
                glGetProgramInfoLog(result.program, sizeof(infoLog), 0, infoLog);
                LogError("Couldn't link shader program\n");
                glDeleteProgram(result.program);
            }
        }
        else
        {
            char infoLog[512];
            glGetShaderInfoLog(fragment_shader_id, sizeof(infoLog), 0, infoLog);
            LogError("Couldn't compile fragment shader program\n");
            glDeleteShader(fragment_shader_id);
        }
    }
    else
    {
        char infoLog[512];
        glGetShaderInfoLog(vertex_shader_id, sizeof(infoLog), 0, infoLog);
        LogError("Couldn't compile vertex shader program\n");
        glDeleteShader(vertex_shader_id);
    }
    return result;
}


// NOTE(fakhri): hotreloading shaders is done by monitoring the /shaders folders and whenever
// some shader changes we detect that change, recompile the shader, and find where in memory the
// old id of that shader was stored, and then we change its value to the new compiled shader 
// effictively hotreloading it

internal
b32 ShaderFileHotreload(M_Arena *arena, s8 changed_shader_path)
{
    b32 result = 0;
    for (u32 metadata_index = 0;
         metadata_index < os->shaders_array.shaders_count;
         ++metadata_index)
    {
        Shader_Metadata *shader_metadata = &os->shaders_array.data[metadata_index];
        if (StringMatch(shader_metadata->shader_path, changed_shader_path))
        {
            // NOTE(fakhri): recompile shader
            Compile_Shader_Result compile_result = CompileShader(arena, changed_shader_path);
            if (compile_result.is_valid)
            {
                result = 1;
                glDeleteProgram(*shader_metadata->shader_id_address);
                *shader_metadata->shader_id_address = compile_result.program;
                if (shader_metadata->setup_callback && !shader_metadata->setup_callback(compile_result.program))
                {
                    BreakDebugger();
                }
            }
            break;
        }
    }
    return result;
}
