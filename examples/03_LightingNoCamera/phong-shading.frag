#version 430 core

layout(location = 0) uniform bool showNormals;

// Information from vertex shader
in vec3 fNormal;
in vec3 fPosition;

// Output color (framebuffer)
out vec4 oColor;

// BSDF configuration
const float n = 128;
const vec3 kd = vec3(0.5); // Gray color
const vec3 ks = vec3(0.5);

void
main()
{
    // Hard coded directional light direction and view
    // Here the light is directional (infinie) 
    // and the camera projective 
    vec3 LightDir = vec3(0,0,1); 
    vec3 EyeDir   = vec3(0,0,1);
    vec3 nNormal  = normalize(fNormal);

    // If we want to show the normals
    // this can be usefull for debugging
    if(showNormals) {
        oColor = vec4(nNormal*0.5 + 0.5, 1.0);
        return;
    }
    
    float cosTheta = dot(nNormal, LightDir);
    if (cosTheta > 0.0)
    {
        // Compute specular (phong)
        vec3 r = reflect(-LightDir, nNormal);
        // Note: max here is to clamp the lobe if it is below the surface
        float specular = pow(max(0.0, dot(EyeDir, r)), n);

        // Compute the material model (specular + diffuse)
        oColor = vec4(vec3(kd * cosTheta + ks * specular), 1.0);
    }
    else
    {
        // Put red color if the light is facing back the surface
        oColor = vec4(1, 0, 0, 1);
    }
}
