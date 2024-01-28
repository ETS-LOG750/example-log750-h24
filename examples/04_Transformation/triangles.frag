#version 430 core

// layout(location = 1) can be removed for lower OpenGL version
layout(location = 1) uniform vec4 uColor;

out vec4 fColor;
void main()
{
    fColor = uColor;
}
