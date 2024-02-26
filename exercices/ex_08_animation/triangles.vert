#version 400 core
uniform mat4 mvMatrix;
in vec4 vPosition;

void
main()
{
     gl_Position = mvMatrix * vPosition;
}

