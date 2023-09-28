#version 460

in vec4 tint;
in vec2 texture_coord;

uniform sampler2D sprite_sheet;

out vec4 colour;

void main()
{
    colour = texture(sprite_sheet, texture_coord);
}
