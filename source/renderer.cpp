internal void
InitRenderer(Renderer *renderer)
{
    // NOTE(fakhri): load shaders
    InitShaderProgram(&renderer->quad_shader, S8Lit("shaders/quad_shader.glsl"));
    InitShaderProgram(&renderer->texture_shader, S8Lit("shaders/texture_shader.glsl"));
    
    // NOTE(fakhri): setup shaders uniforms
    {
        renderer->screen = vec2iv(os->window_size);
        
        m4 ortho_projection = m4_orthographic(0.0f, renderer->screen.width, 
                                              renderer->screen.height, 0.0f,
                                              0.f, 100.f);
        
        // NOTE(fakhri): quad_shader uniforms
        {
            glUseProgram(renderer->quad_shader);
            GLint projection_location = glGetUniformLocation(renderer->quad_shader, "projection");
            glUniformMatrix4fv(projection_location, 1, GL_FALSE, (f32*)&ortho_projection);
        }
        
        // NOTE(fakhri): quad_shader uniforms
        {
            glUseProgram(renderer->texture_shader);
            GLint projection_location = glGetUniformLocation(renderer->texture_shader, "projection");
            glUniformMatrix4fv(projection_location, 1, GL_FALSE, (f32*)&ortho_projection);
        }
        
        glUseProgram(0);
    }
    
    f32 quad_vertices[] = {
        -0.5f, +0.5f,
        +0.5f, +0.5f,
        +0.5f, -0.5f,
        
        -0.5f, +0.5f,
        +0.5f, -0.5f,
        -0.5f, -0.5f,
    };
    
    glGenVertexArrays(1, &renderer->quad_vao);
    glBindVertexArray(renderer->quad_vao);
    
    glGenBuffers(1, &renderer->quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->quad_vbo);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), (void*)0);
    glEnableVertexAttribArray(0);
    
    f32 texture_vertices[] = {
        // position         texture
        -0.5f, -0.5f, 	 0.0f, 0.0f,
        +0.5f,  0.5f, 	 1.0f, 1.0f,
        -0.5f,  0.5f, 	 0.0f, 1.0f,
        
        -0.5f, -0.5f, 	 0.0f, 0.0f,
        +0.5f, -0.5f, 	 1.0f, 0.0f,
        +0.5f,  0.5f, 	 1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &renderer->texture_vao);
    glBindVertexArray(renderer->texture_vao);
    
    glGenBuffers(1, &renderer->texture_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->texture_vbo);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(texture_vertices), texture_vertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void *)0);
    
    // NOTE(fakhri): load textures
    {
        s8 image_path = S8Lit("images/take_my_money.jpg");
        
        glGenTextures(1, &renderer->test_2d_texture);
        glBindTexture(GL_TEXTURE_2D, renderer->test_2d_texture);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        i32 nb_channels, tex_width, tex_height;
        void* data = stbi_load(image_path.str, &tex_width, &tex_height, &nb_channels, 0);
        if (data)
        {
            GLenum format = 0;
            switch(nb_channels)
            {
                case 1: format = GL_RED; break;
                case 3: format = GL_RGB; break;
                case 4: format = GL_RGBA; break;
                default: LogWarning("channels count %d not handled when loading image %s", nb_channels, image_path.str);
            }
            
            glTexImage2D(GL_TEXTURE_2D, 0, format, tex_width, tex_height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
            
        }
        else
        {
            LogWarning("couldn't load image %s", image_path.str);
        }
    }
}

internal
void DebugDrawQuad(Renderer *renderer, v2 pos, v2 size, v3 color, f32 z_rotation = 0.f)
{
    f32 window_width = (f32)os->window_size.width;
    f32 window_height = (f32)os->window_size.height;
    
    pos = v2{pos.x + renderer->screen.width / 2, renderer->screen.height / 2 - pos.y};
    
    m4 trans = m4_translate(vec3(pos, 0.f));
    m4 rotat = m4_translate(vec3(-size / 2, 0.f)) * m4_rotate(-z_rotation, v3{0.f, 0.f, 1.f}) * m4_translate(vec3(size / 2, 0.f));
    m4 scale = m4_scale(vec3(size, 1.0f));
    m4 model = trans * rotat * scale;
    
    glUseProgram(renderer->quad_shader);
    GLint model_location = glGetUniformLocation(renderer->quad_shader, "model");
    glUniformMatrix4fv(model_location, 1, GL_FALSE, (f32*)&model);
    GLint color_location = glGetUniformLocation(renderer->quad_shader, "color");
    glUniform4f(color_location, color.r, color.g, color.b, 1.0f);
    
    glBindVertexArray(renderer->quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glUseProgram(0);
}

internal
void DebugDrawTexture(Renderer *renderer, GLuint tex, v2 pos, v2 size)
{
    f32 window_width = (f32)os->window_size.width;
    f32 window_height = (f32)os->window_size.height;
    
    // NOTE(fakhri): the origin at the center of the screen
    pos = v2{pos.x + window_width / 2, window_height / 2 - pos.y};
    m4 trans = m4_translate(vec3(pos, 0.f));
    m4 scale = m4_scale(vec3(size, 1.0f));
    m4 model = trans * scale;
    
    glUseProgram(renderer->texture_shader);
    GLint model_location = glGetUniformLocation(renderer->quad_shader, "model");
    glUniformMatrix4fv(model_location, 1, GL_FALSE, (f32*)&model);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glBindVertexArray(renderer->texture_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glUseProgram(0);
}
