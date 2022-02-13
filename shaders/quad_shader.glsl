
#ifdef VERTEX_SHADER
layout (location = 0) in vec2 position_attrib;
uniform mat4 model;
uniform mat4 projection;
void main()
{
    gl_Position = projection * model * vec4(position_attrib, 0.0, 1.0);
}
#endif

#ifdef FRAGMENT_SHADER
out vec4 frag_color;
uniform vec4 color;
void main()
{
    frag_color = color;
}
#endif
