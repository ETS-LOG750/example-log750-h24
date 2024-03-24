#version 400 core

uniform mat4 mvMatrix;
uniform mat4 projMatrix;

in vec4 vPosition;
in vec3 vNormal;

out vec3 fNormalWorld;
out vec3 fPositionWorld;

void main()
{
     vec4 vEyeCoord = mvMatrix * vPosition;
     gl_Position = projMatrix * vEyeCoord;

     // Direction de vue (pour la reflexion)
     // en espace monde
     fPositionWorld = vPosition.xyz;
     fNormalWorld = vNormal; 
}

