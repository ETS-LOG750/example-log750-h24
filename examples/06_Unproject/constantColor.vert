#version 400 core
uniform mat4 uMatrix;
uniform mat4 uProjMatrix;

in vec4 vPosition;

void main()
{
  gl_Position = uProjMatrix * uMatrix * vPosition;
}

