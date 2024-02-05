#version 430 core

layout(location = 0) uniform mat4 uMatrix;
layout(location = 1) uniform mat3 uMatrixNormal;

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;

out vec3 fNormal;

void main()
{
    // Output (pour le fragment shader)
    fNormal = uMatrixNormal * vNormal;

    // Sortie dans l'espace NDC (normalized device coordinate)
    vec3 fPosition = vec3(uMatrix * vPosition);
    gl_Position = vec4(fPosition, 1.0); // La projection sera effectue dans le geometry shader
}

