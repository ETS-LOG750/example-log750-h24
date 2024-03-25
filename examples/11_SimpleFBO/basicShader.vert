#version 430 core

layout(location = 0) uniform mat4 mvMatrix;
layout(location = 1) uniform mat4 projMatrix;
layout(location = 2) uniform mat3 normalMatrix;

in vec4 vPosition;
in vec3 vNormal;

out vec3 fNormal;
out vec3 fPosition;

void
main()
{
     vec4 vEyeCoord = mvMatrix * vPosition;
     gl_Position = projMatrix * vEyeCoord;

     fPosition = vEyeCoord.xyz;
     fNormal = normalMatrix*vNormal;
}

