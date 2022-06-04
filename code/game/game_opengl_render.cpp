
internal void
OpenGL_DrawImage(Shader_Program *program, Texture2D texture, b32 flip_y, m4 projection, m4 model, v4 src, v4 color)
{
    glUseProgram(program->id);
    
    // NOTE(fakhri): sending uniforms to the GPU
    {
        glUniformMatrix4fv(glGetUniformLocation(program->id, "projection"), 1, GL_FALSE, (f32*)&projection);
        glUniformMatrix4fv(glGetUniformLocation(program->id, "model"), 1, GL_FALSE, (f32*)&model);
        glUniform4fv(glGetUniformLocation(program->id, "src"), 1, (f32 *)&src);
        glUniform4fv(glGetUniformLocation(program->id, "tilting_color"), 1, (f32 *)&color);
        glUniform1i(glGetUniformLocation(program->id, "flip_y"), flip_y);
    }
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glBindVertexArray(program->vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
}

internal void
OpenGL_DrawSolidQuad(Shader_Program *program, m4 projection, m4 model, v4 color)
{
    glUseProgram(program->id);
    
    // NOTE(fakhri): sending uniforms to the GPU
    {
        glUniformMatrix4fv(glGetUniformLocation(program->id, "projection"), 1, GL_FALSE, (f32*)&projection);
        glUniformMatrix4fv(glGetUniformLocation(program->id, "model"), 1, GL_FALSE, (f32*)&model);
        glUniform4fv(glGetUniformLocation(program->id, "color"), 1, (f32*)&color);
    }
    
    glBindVertexArray(program->vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
}

internal void
OpenGL_Clear(v4 color)
{
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}
