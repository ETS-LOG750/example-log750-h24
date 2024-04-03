#version 400 core

uniform sampler2D texShadowMap;
uniform vec4 uColor;

uniform vec3 lightPositionCameraSpace;

uniform int biasType;
uniform float biasValue;
uniform float biasValueMin;

in vec3 fNormal;
in vec3 fPosition;
in vec4 fShadowCoord;

out vec4 oColor;

void main()
{
    vec3 LightDirection = normalize(lightPositionCameraSpace-fPosition);
    float diffuse = max(0.0, dot(fNormal, LightDirection));

    vec4 materialColor = uColor;

    // Project the shadow coordinate by dividing by parameter w.
    //
    // We also need to map uv coordinates from NDC to the range [0...1, 0...1]
    //  and depth values (z) from NDC to the range [0...1]
    //
    vec3 coord = 0.5*(fShadowCoord.xyz / fShadowCoord.w)+0.5;

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(texShadowMap, coord.xy).r; 

    // get depth of current fragment from light's perspective
    float currentDepth = coord.z;

    float bias = 0.0; 
    if(biasType == 1) {
        bias = biasValue;
    } else if(biasType == 2) {
        bias = max(biasValue * (1.0 - dot(fNormal, LightDirection)), biasValueMin); 
    }
    

    // check whether current frag pos is in shadow
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

    // Diffuse and ambient lighting.
    vec4 ambient = vec4(vec3(0.1),1.0);
    oColor = (1-shadow) * materialColor * diffuse + ambient;
}
