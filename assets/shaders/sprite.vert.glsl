#version 460

layout (location = 0) in vec2 vertex;
layout (location = 1) in vec4 colour;
layout (location = 2) in vec2 uv;

uniform mat4 projection;
uniform mat4 model;

out vec4 tint;
out vec2 texture_coord;

void main()
{
    gl_Position = projection * vec4(vertex, 0, 1);
    tint = colour;
	texture_coord = uv;
}
