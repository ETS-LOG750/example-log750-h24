#version 400 core

in vec3 fNormal;
in vec3 fPosition;

out vec4 oColor;

uniform vec3 diffuseColor;
uniform vec3 pointIntensity;

void main()
{
    // Materials
    vec3 Kd = diffuseColor;
    vec3 Ks = vec3(0.0);
    float n = 128;
   
    // Local model
    vec3 nViewDirection = normalize(vec3(0.0)-fPosition);
    vec3 nNormal = normalize(fNormal);
    
    // ========== Light contribution computation
    vec3 contrib = vec3(0.0,0.0,0.0);
    
    // Light 1 (point light)
    vec3 lightPosition = vec3(0,0,0);
    vec3 nLightDirection = normalize(lightPosition-fPosition);

    // Distance
    float distSqr = dot(fPosition, fPosition);
    float diffuse = dot(nNormal, nLightDirection);
    if (diffuse > 0.0)
    {
        vec3 reflectL = normalize(-nLightDirection+2.0*nNormal*dot(nNormal,nLightDirection));
        float specular = pow(max(0.0, dot(nViewDirection, reflectL)), n);
        contrib += (Kd * diffuse + Ks * specular) * pointIntensity;
    }

    oColor = vec4(contrib, 1.0);
}