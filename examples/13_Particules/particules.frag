#version 430

in vec2 ex_TexCoor;
in vec3 ex_color;
out vec4 color;

uniform float globalTransparency;

uniform sampler2D texture;
uniform bool useTexture;
uniform float time; // Temps de la simulation

void main(void){
    vec4 outputColor = vec4(ex_color, globalTransparency);
    if(useTexture) {
        outputColor = texture2D(texture, ex_TexCoor);
        
        // Play around with the values to change color of particles over time
        // https://github.com/StanEpp/OpenGL_ParticleSystem
        float green  = cos(time * 0.2 + 1.5) + 1.f;
        float red = cos(time * 0.04) * sin(time * 0.003) * 0.35 + 1.f;
        float blue = sin(time * 0.0006) * 0.5 + 1.f;

        outputColor.x *= red;
        outputColor.y *= green;
        outputColor.z *= blue;
    }

    color = outputColor;
}