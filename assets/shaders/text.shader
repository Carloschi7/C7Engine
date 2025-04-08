//screen.shader is just a shader that draws stuff on the screen interface
//caring only about the application projection matrix and not about the
//current view. This is perfect for drawing instanced console text and much more

#shader vertex
#version 430 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 tex_coords;

uniform mat4 proj;

out vec2 TexCoords;

void main()
{
	TexCoords = tex_coords;
	gl_Position = proj * vec4(pos, 0.0f, 1.0f);
}

#shader fragment
#version 430 core

in vec2 TexCoords;
uniform sampler2D glyph_texture;

out vec4 FragColor;

void main()
{
	vec3 texture_color  = texture(glyph_texture, TexCoords).rgb;
	if(texture_color.r < 0.3f) discard;
	FragColor = vec4(0.0f, 1.0f, 0.0f, 0.8f);
}