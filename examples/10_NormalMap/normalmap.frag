#version 400 core
uniform sampler2D texColor;
uniform sampler2D texNormal;
uniform sampler2D texARM;

uniform bool activateARM;
uniform bool activateNormalMap;
uniform vec3 lightDirection;

in vec2 fUV;
in vec3 fNormal;
in vec3 fTangent;
in vec3 fBitangent;
in vec3 fPosition;


// Out shading color
out vec4 fColor;

void
main()
{
    // Build the matrix to transform from XYZ (normal map) space to TBN (tangent) space
    // Each vector fills a column of the matrix
    vec3 normal;
    if(activateNormalMap) {
        mat3 tbn = mat3(normalize(fTangent), normalize(fBitangent), normalize(fNormal));
        vec3 normalFromTexture = texture(texNormal, fUV).rgb * 2.0 - vec3(1.0);
        normal = normalize(tbn * normalFromTexture);
    } else {
        normal = normalize(fNormal);
    }
    
    vec3 nViewDirection = normalize(-fPosition); // As position is expressed in camera space

    // Read AO, Roughness and Metallic texture
    float ao = 0;
    float n = 32;
    float propSpec = 0.5;
    if(!activateARM) {
        vec3 ARM = texture(texARM, fUV).rgb;
        ao = ARM.r;
        // Convert roughness to phong exponent
        // http://simonstechblog.blogspot.com/2011/12/microfacet-brdf.html
        n = sqrt(2.0/(ARM.g+2));
        propSpec = ARM.b;
       
    }    

    // Shading
    float cosTheta = max(0.0, dot(normal, lightDirection));
    if(cosTheta != 0.0) {
        vec3 Rl = normalize(-lightDirection+2.0*normal*cosTheta);
        float specular = pow(max(0.0, dot(Rl, nViewDirection)), n);
        vec4 Kd = texture(texColor, fUV);
        fColor = ao*0.2 + Kd * cosTheta * (1-propSpec) + vec4(1.0) * specular * propSpec;
    } else {
        fColor = vec4(vec3(0.0), 0.0);
    }
}
