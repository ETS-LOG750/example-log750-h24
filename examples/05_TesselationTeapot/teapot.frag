#version 430 core

layout(location = 3) uniform vec4 uColor;
uniform bool showNormal;

in vec3 fNormal;

out  vec4 oColor;

void
main()
{
    if(showNormal) {
        oColor = vec4(normalize(fNormal) * 0.5 + 0.5, 1.0);
    } else {
       oColor = uColor;
    }
    
}
