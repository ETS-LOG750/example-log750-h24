#version 460

struct Particle{
    vec3 position;
    float life;
    vec3 velocity;
    float size;
    vec3 color;
    float _pad;
};

layout(binding = 0, std430) readonly buffer ssbo1 {
    Particle data[];
};

uniform float globalSize;

out float quadLength;
out vec3 quadColor;

void main(void){
    vec4 pPos = vec4(data[gl_VertexID].position, 1.0);
    float pSize = data[gl_VertexID].size;
    vec3 pColor = data[gl_VertexID].color;

    gl_Position = pPos;
    quadLength = pSize * globalSize;
    quadColor = pColor;
}