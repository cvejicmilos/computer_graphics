#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 lightProj;
uniform mat4 lightTransforms[6];

out vec4 gFragPos;
in vec2 vUV[];
out vec2 gUV;

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face;
        for(int i = 0; i < 3; ++i)
        {
            gUV = vUV[i];
            gFragPos = gl_in[i].gl_Position;
            gl_Position = (lightProj * lightTransforms[face]) * gFragPos;
            EmitVertex();
        }    
        EndPrimitive();
    }
}  