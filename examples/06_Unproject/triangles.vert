#version 400 core

uniform mat4 uMatrix;
uniform mat4 uProjMatrix;
uniform mat3 uNormalMatrix;

in vec4 vPosition;
in vec4 vColor;
in vec3 vNormal;

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

