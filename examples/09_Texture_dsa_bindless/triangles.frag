#version 460 core
#extension GL_ARB_bindless_texture : require 

// Texture
layout(bindless_sampler) uniform sampler2D texDiffuse;
layout(bindless_sampler) uniform sampler2D texARM;

uniform bool activateARM;

in vec2 fUV;
in vec3 fNormal;
in vec3 fViewDirection;
in vec3 fPosition;

out vec4 fColor;

void
main()
{
    // Light source on camera position (no decrease with the distance)
    vec3 LightDirection = normalize(vec3(0.0)-fPosition);
    vec3 nfNormal = normalize(fNormal);
    vec3 nviewDirection = normalize(fViewDirection);

    // Compute cosTheta
    float cosTheta = dot(nfNormal, LightDirection);

    // Retrive color from the texture
    vec4 Kd = texture(texDiffuse, fUV);
    vec3 ARM = texture(texARM, fUV).rgb;
    if(!activateARM) {
        ARM = vec3(0.0);
    }

    // Convert roughness to phong exponent
    // http://simonstechblog.blogspot.com/2011/12/microfacet-brdf.html
    float n = sqrt(2.0/(ARM.g+2));

    if (cosTheta >= 0)
    {
        // Reflexion
        vec3 Rl = normalize(-LightDirection+2.0*nfNormal*dot(nfNormal,LightDirection));
        float specular = pow(max(0.0, dot(Rl, nviewDirection)), n);

        // Use phong model with ambiant and controlled metallic
        // the diffuse color is directly control with diffuse texture (Kd)
        fColor = ARM.r*0.2 + Kd * cosTheta * (1-ARM.b) + vec4(1.0) * specular * (ARM.b);
    }
    else
    {
        // Show the texture (arbitrary in this case)
        fColor = Kd;
    }
}
