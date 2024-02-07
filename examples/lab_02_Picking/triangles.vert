#version 400 core

uniform mat4 uMatrix;
uniform mat4 uProjMatrix;
uniform mat3 uNormalMatrix;

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec4 vColor;


out vec4 ifColor;
out vec3 fNormal;
out vec3 fPosition;

void
main()
{
     vec4 vEyeCoord = uMatrix * vPosition;
     gl_Position = uProjMatrix * vEyeCoord;
     fPosition = vEyeCoord.xyz;
     fNormal = uNormalMatrix*vNormal;
     ifColor = vColor;
}

