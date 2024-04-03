#version 400 core

uniform mat4 MVMatrix;
uniform mat4 ProjMatrix;
uniform mat4 MLPMatrix;
uniform mat3 normalMatrix;
uniform vec4 uColor;

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;

out vec3 fNormal;
out vec3 fPosition;
out vec4 fShadowCoord;

void main()
{
     vec4 vEyeCoord = MVMatrix * vPosition;
     gl_Position = ProjMatrix * vEyeCoord;

     fPosition = vEyeCoord.xyz;
     fNormal = normalMatrix * vNormal;

     // Project inside shadow map
     fShadowCoord = MLPMatrix * vPosition;
}
