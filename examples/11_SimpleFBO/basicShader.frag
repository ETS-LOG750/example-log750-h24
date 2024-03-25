#version 430 core

layout(location = 3) uniform vec3 Kd;
layout(location = 4) uniform vec3 Ks;
layout(location = 5) uniform float Kn;
layout(location = 6) uniform vec3 lightPos;

in vec3 fNormal;
in vec3 fPosition;

layout(location = 0) out vec4 oColor;
layout(location = 1) out vec3 oPosition;
void main()
{
    // Get lighting vectors
    vec3 LightDirection = normalize(lightPos-fPosition);
    vec3 nfNormal = normalize(fNormal);
    vec3 nviewDirection = normalize(vec3(0.0)-fPosition);

    // Compute diffuse component
    vec3 diffuse = Kd * max(0.0, dot(nfNormal, LightDirection));

    // Compute specular component
    vec3 Rl = normalize(-LightDirection+2.0*nfNormal*dot(nfNormal,LightDirection));
    vec3 specular = Ks*pow(max(0.0, dot(Rl, nviewDirection)), Kn);

    // Compute final color
    oColor = vec4(diffuse +  specular, 1);
    oPosition = fPosition;
}
