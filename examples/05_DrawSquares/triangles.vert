#version 400 core

uniform mat4 uMatrix;

in vec4 vPosition;

void main()
{
    vec4 p = uMatrix * vPosition;
    gl_Position = vec4(p.xy, -p.z, p.w);
}

