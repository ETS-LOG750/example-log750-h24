#version 400 core

in vec3 fNormal;

out vec4 fColor;
void main()
{
    vec3 nNormal = normalize(fNormal);
    fColor = vec4(nNormal*0.5 + 0.5, 1.0);
}
