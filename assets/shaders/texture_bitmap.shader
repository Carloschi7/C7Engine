#shader vertex
#version 330 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 tex_coord;

uniform mat4 proj;
uniform mat4 model;
out vec2 TexCoord;

void main() {
	TexCoord = tex_coord;
	gl_Position = proj * model * vec4(pos, 0.0f, 1.0f);
}

#shader fragment
#version 330 core

in vec2 TexCoord;
uniform sampler2D bitmap;
uniform float xoffset;
uniform float yoffset;
out vec4 FragColor;

void main() {
	vec2 coords = vec2(TexCoord.x + xoffset, TexCoord.y + yoffset);
	FragColor = texture(bitmap, coords);
}