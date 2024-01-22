#version 430 core

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;

out vec3 fColor;

// BSDF configuration
const float n = 128;
const vec3 kd = vec3(0.5);
const vec3 ks = vec3(0.5);

void
main()
{
    vec3 LightDirection = vec3(0,0,1); 
    vec3 EyeDirection = vec3(0,0,1);
    vec3 nNormal = normalize(vNormal);

    float diffuse = dot(nNormal, LightDirection);
    if (diffuse > 0.0)
    {
        vec3 r = reflect(-LightDirection, nNormal);
        float specular = pow(max(0.0, dot(EyeDirection, r)), n);
        fColor = vec3(kd * diffuse + ks * specular);
    }
    else
    {
        fColor = vec3(1, 0, 0);
    }


     gl_Position = vec4(vPosition.x, vPosition.y, -vPosition.z, vPosition.w);
}

