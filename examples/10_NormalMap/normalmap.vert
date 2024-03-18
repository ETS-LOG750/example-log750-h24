#version 400 core

uniform mat4 mvMatrix;
uniform mat4 projMatrix;
uniform mat3 normalMatrix;

in vec4 vPosition;
in vec3 vNormal;
in vec3 vTangent;
in vec2 vUV;

out vec2 fUV;
out vec3 fNormal;
out vec3 fTangent;
out vec3 fBitangent;
out vec3 fPosition;

void
main()
{
     vec4 vEyeCoord = mvMatrix * vPosition;
     gl_Position = projMatrix * vEyeCoord;

     // Vertex position
     fPosition = vEyeCoord.xyz;
     
     // Compute normal information
     fNormal = normalize(normalMatrix*vNormal);
     fTangent = normalize(mat3(mvMatrix)*vTangent); 

     // re-orthogonalize tangent with respect to N
     // to make sure that the two vectors are orthogonal
     fTangent = normalize(fTangent - dot(fTangent, fNormal) * fNormal);

     // As the two vectors are orthogonal, we can compute the bitangent vector from cross product
     fBitangent = cross(fNormal, fTangent);

     // Texture coordinates
     fUV = vUV;
}

