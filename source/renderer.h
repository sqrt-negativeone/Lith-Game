/* date = January 29th 2022 11:29 am */

#ifndef RENDERER_H
#define RENDERER_H

struct Renderer
{
    GLuint quad_shader;
    GLuint texture_shader;
    
    GLuint quad_vao;
    GLuint quad_vbo;
    
    GLuint texture_vao;
    GLuint texture_vbo;
    
    v2 screen;
    
    GLuint test_2d_texture;
};

#endif //RENDERER_H
