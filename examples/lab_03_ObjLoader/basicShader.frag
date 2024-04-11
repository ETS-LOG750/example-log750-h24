#version 400 core
uniform vec3 Kd;
uniform vec3 Ks;
uniform float Kn;
uniform vec3 lightPos;

in vec3 fNormal;
in vec3 fPosition;

out vec4 fColor;
void
main()
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
    fColor = vec4(diffuse +  specular, 1);
}
