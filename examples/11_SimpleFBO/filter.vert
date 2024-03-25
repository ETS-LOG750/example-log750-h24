#version 430 core

// Assume that the position are expressed in canonical space
in vec3 vPosition;
in vec2 vUV;

// Interpolation of UV coordinates
out vec2 fUV;

void main()
{
     gl_Position = vec4(vPosition, 1);
     fUV = vUV;
}
