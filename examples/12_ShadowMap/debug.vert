#version 400 core

layout(location = 0) in vec4 vPosition;

out vec2 fUV;

void main()
{
    gl_Position = vPosition;
    fUV = (vPosition.xy)*0.5 + 0.5;
}
