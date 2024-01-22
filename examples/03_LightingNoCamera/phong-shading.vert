#version 430 core

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;

out vec3 fNormal;
out vec3 fPosition;

void
main()
{
     // Information interpolee dans le fragment shader
     fPosition = vPosition.xyz;
     fNormal = vNormal;

     // Sortie de la position dans le NDC
     gl_Position = vec4(vPosition.x, vPosition.y, -vPosition.z, vPosition.w);
}

