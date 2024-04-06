#version 430

layout(location=0) in vec4 pPos;
layout(location=1) in float pSize;
layout(location=2) in vec3 pColor;

uniform float globalSize;

out float quadLength;
out vec3 quadColor;

void main(void){
    gl_Position = pPos;
    quadLength = pSize * globalSize;
    quadColor = pColor;
}