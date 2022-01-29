
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
uniform sampler2D image;

out vec4 frag_color;

void main()
{
	frag_color = texture(image, tex_coord);
}

#endif
