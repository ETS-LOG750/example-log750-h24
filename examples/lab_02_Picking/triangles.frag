#version 400 core

in vec4 ifColor;
in vec3 fNormal;
in vec3 fPosition;

out vec4 fColor;

void
main()
{
    // Get lighting vectors
    vec3 LightDirection = normalize(vec3(0.0)-fPosition);	// We assume light is at camera position
    vec3 nfNormal = normalize(fNormal);
    vec3 nviewDirection = normalize(vec3(0.0)-fPosition);

    // Compute diffuse component
    float diffuse = max(0.0, abs(dot(nfNormal, LightDirection)));

    // Compute specular component
    vec3 Rl = normalize(-LightDirection+2.0*nfNormal*dot(nfNormal,LightDirection));
    float specular = pow(max(0.0, dot(Rl, nviewDirection)), 128);

    // Compute final color
    fColor = ifColor * diffuse + vec4(vec3(0.5), 1.0) * specular;
}
