#version 400 core
in vec3 pos;
void
main()
{
     // Position needs to be in [-1,1]^3
     // Otherwise, the some part of the primitive will be clipped
     gl_Position = vec4(pos, 1.0);
}

