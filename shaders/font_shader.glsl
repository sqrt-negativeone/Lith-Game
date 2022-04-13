
#ifdef VERTEX_SHADER
layout (location = 0) in vec4 vertex;

out vec2 tex_coord;

uniform mat4 projection;
uniform mat4 model;

void main()
{
    tex_coord= vertex.zw;
	gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}
#endif


#ifdef FRAGMENT_SHADER

in vec2 tex_coord;
uniform sampler2D font_texture;

uniform vec4 glyph_src;
uniform vec4 color;
out vec4 frag_color;


void main()
{
	vec2 glyph_start = glyph_src.xy;
    vec2 glyph_end = glyph_src.zw;
    
    vec2 glyph_position = mix(glyph_start, glyph_end, tex_coord);
    
    frag_color = color * texture(font_texture, glyph_position);
    
    if (frag_color.a < 0.001) discard;
}

#endif
