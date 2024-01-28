#version 430 core

// layout(location = 0) can be removed for lower OpenGL version
layout(location = 0) uniform mat4 uMatrix;

// Vertex informations
in vec4 vPosition;

void main()
{
    vec4 p = uMatrix * vPosition;
    gl_Position = vec4(p.x, p.y, -p.z, p.w);
}

