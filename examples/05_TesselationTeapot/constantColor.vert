#version 430 core

layout(location = 0) uniform mat4  MV;
layout(location = 2) uniform mat4  P;

layout(location = 0) in vec4 vPosition;

void main()
{
  gl_Position = P * MV * vPosition;
}

