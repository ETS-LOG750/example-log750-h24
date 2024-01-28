#version 400 core
uniform mat4 uMatrix;
in vec3 vPosition;

void
main()
{
     // Scale down for better visualiation
     mat4 matrix = uMatrix;
     mat4 scale = mat4(1.0);
     scale[0][0] = 1.0 / 5.0;
     scale[1][1] = 1.0 / 5.0;
     
     matrix = scale * matrix;
     gl_Position = matrix *  vec4(vPosition, 1.0);
}

