#version 400 core

// model-light-projection matrix
uniform mat4 MLP;

// input vertex position
layout(location = 0) in vec4 vPosition;           

void main()
{
    gl_Position = MLP * vPosition;
}
