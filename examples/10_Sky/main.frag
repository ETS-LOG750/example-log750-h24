#version 400 core

uniform vec3 cameraPos;
uniform sampler2D texSkydome;
uniform bool useFresnel; 

in vec3 fNormalWorld;
in vec3 fPositionWorld;

out vec4 oColor;

#define PI 3.14159265358979323844

void main()
{
    // Local model
    vec3 viewDirectionWorld = normalize(cameraPos - fPositionWorld);
    vec3 normalWorld = normalize(fNormalWorld);

    // Do a reflection
    vec3 r = reflect(-viewDirectionWorld, normalWorld);

    // Calcul des coordonnees UV pour lire la carte d'env
    float phi=atan(r.z, r.x)* 0.5 / PI + 0.5;
    float theta = 1.0 - acos(r.y) / PI;
    vec2 uv = vec2(phi, theta);

  
    float fresnel = 1.0;
    if(useFresnel) {
        // Add fresnel term for more realism
        // https://graphicscompendium.com/raytracing/11-fresnel-beer 
        // This is optional 
        float eta = 1.3;
        float f0 = ((eta - 1)*(eta - 1)) / ((eta + 1) * (eta + 1));
        float cosTheta = dot(r, normalWorld);
        fresnel = f0 + (1 - f0) * pow(1 - cosTheta, 5);
    }

    // Couleur finale
    oColor = fresnel * texture(texSkydome, uv);
}