#version 400 core

uniform sampler2D tex;
uniform float scale;

in vec2 fUV;

out vec4 oColor;

void main()
{
    oColor = texture(tex, fUV).rrrr * scale;
}
