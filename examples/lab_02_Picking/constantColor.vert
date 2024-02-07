#version 400 core

uniform mat4 uMatrix;
uniform mat4 uProjMatrix;

layout(location = 0) in vec4 vPosition;

void
main()
{
  gl_Position = uProjMatrix * uMatrix * vPosition;
}

