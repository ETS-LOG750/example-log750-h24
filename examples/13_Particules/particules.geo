#version 430

// From: https://github.com/StanEpp/OpenGL_ParticleSystem

// Entree des position des particules
// Sortie un quad oriente par rapport a la camera
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

// Entree: taille differente des particules
in float quadLength[];
in vec3 quadColor[];

// Sortie: coordonnee de texture
out vec2 ex_TexCoor;
out vec3 ex_color;

// Matrices de projections (pour transformer les sommets)
uniform mat4  viewMatrix;
uniform mat4  projMatrix;

void main(void){
    // Expression de la position dans l'espace de camera
    vec4 normal = normalize(viewMatrix * gl_in[0].gl_Position);
    // Construction du repere
    vec3 rightAxis  = cross(normal.xyz, vec3(0,1,0));
    vec3 upAxis   = cross(rightAxis, normal.xyz);

    // Calcul des sommets
    vec4 rightVector  = projMatrix * vec4(rightAxis.xyz, 1.0f) * (quadLength[0]*0.5f);
    vec4 upVector     = projMatrix * vec4(upAxis.xyz, 1.0f) * (quadLength[0]*0.5f);
    vec4 particlePos  = projMatrix * viewMatrix * gl_in[0].gl_Position;

    // Generation de la particule
    gl_Position = particlePos-rightVector - upVector;
    ex_TexCoor = vec2(0,0);
    ex_color = quadColor[0];
    EmitVertex();

    gl_Position = particlePos+rightVector - upVector;
    gl_Position.x += 0.5*quadLength[0];
    ex_TexCoor = vec2(1,0);
    ex_color = quadColor[0];
    EmitVertex();

    gl_Position = particlePos-rightVector + upVector;
    gl_Position.y += 0.5*quadLength[0];
    ex_TexCoor = vec2(0,1);
    ex_color = quadColor[0];
    EmitVertex();

    gl_Position = particlePos+rightVector + upVector;
    gl_Position.y += 0.5*quadLength[0];
    gl_Position.x += 0.5*quadLength[0];
    ex_TexCoor = vec2(1,1);
    ex_color = quadColor[0];
    EmitVertex();
}