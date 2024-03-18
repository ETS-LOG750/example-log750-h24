#version 400 core
uniform sampler2D texSkydome;

in vec3 fPosition;

// Out shading color
out vec4 oColor;

#define PI 3.14159265358979323844

void
main()
{
    // Normalisation de la position (e.g., direction) exprimer en espace monde
    vec3 nPos = normalize(fPosition);

    // Calcul des coordonnees UV
    float phi=atan(nPos.z, nPos.x)* 0.5 / PI + 0.5;
    float theta = 1.0 - acos(nPos.y) / PI;
    vec2 uv = vec2(phi, theta);

    // Couleur finale
    oColor = texture(texSkydome, uv);
}
