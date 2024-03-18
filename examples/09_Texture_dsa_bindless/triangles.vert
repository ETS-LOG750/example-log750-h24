#version 460 core
uniform mat4 mvMatrix;
uniform mat4 projMatrix;
uniform mat3 normalMatrix;

in vec4 vPosition;
in vec2 vUV;
in vec3 vNormal;

out vec2 fUV;
out vec3 fNormal;
out vec3 fViewDirection;
out vec3 fPosition;

void main()
{
     vec4 vEyeCoord = mvMatrix * vPosition;

     fPosition = vEyeCoord.xyz;
     fNormal = normalMatrix*vNormal;
     fUV = vUV;
     fViewDirection = -vEyeCoord.xyz;

     gl_Position = projMatrix * vEyeCoord;
}

