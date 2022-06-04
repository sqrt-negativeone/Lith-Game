
#ifdef VERTEX_SHADER
layout (location = 0) in vec4 vertex;
out vec2 tex_coord;
uniform mat4 projection;
uniform mat4 model;

void main()
{
    tex_coord = vertex.zw;
	gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}
#endif


#ifdef FRAGMENT_SHADER

in vec2 tex_coord;
uniform sampler2D tex;
uniform vec4 src;
uniform vec4 tilting_color;
uniform int flip_y = 0;
out vec4 frag_color;

void main()
{
    vec2 sampled_pos = mix(src.xy, src.zw, tex_coord);
    sampled_pos.y = (bool(flip_y))? 1 - sampled_pos.y: sampled_pos.y;
    
    frag_color = tilting_color * texture(tex, sampled_pos);
}

#endif
