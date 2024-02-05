#version 430 core

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

layout(location = 2) uniform float uScale;

in vec3 fNormal[];

// Other example
void GenerateLine(int index)
{
    // Right to left handed
    // All the primitives needs to be properly generated
    vec4 projection = vec4(1, 1, -1, 1);

    gl_Position = projection * gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = projection * (gl_in[index].gl_Position + 
                                vec4(fNormal[index], 0.0) * uScale);
    EmitVertex();
    EndPrimitive();
}

void main()
{
    GenerateLine(0); // first vertex normal
    GenerateLine(1); // second vertex normal
    GenerateLine(2); // third vertex normal
}  