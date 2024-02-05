#version 430 core

layout(location = 3) uniform vec4 uColor;

out vec4 fColor;

void main()
{
  fColor = uColor;
}
