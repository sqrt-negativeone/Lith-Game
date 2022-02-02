
#ifdef VERTEX_SHADER
layout (location = 0) in vec4 vertex;

uniform mat4 projection;
out vec2 tex_position;

void main()
{
	tex_position = vertex.zw;
	gl_Position = projection * vec4(vertex.xy, 0, 1.0);
}

#endif


#ifdef FRAGMENT_SHADER

in vec2 tex_position;
out vec4 frag_color;

uniform vec3 text_color;
uniform sampler2D character;

void main()
{
	vec4 sampled_color = vec4(1.0, 1.0, 1.0, texture(character, tex_position).r);
	frag_color = vec4(text_color, 1.0) * sampled_color;
}

#endif
