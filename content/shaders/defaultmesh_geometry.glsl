#version 420 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

// World space attributes
in VertexAttrib
{
    vec3 position; // discarded
    vec3 normal;
    vec4 color;
    vec4 tcoord;
} vertex[];

// World space attributes
out VertexAttrib
{
    vec3 position;
    vec3 normal;
    vec4 color;
    vec4 tcoord;
} vertexout;

uniform bool useFlatShading;

void main()
{
    if (useFlatShading)
    {
        // Flat face shading
        vec3 v1 = vertex[1].position - vertex[0].position;
        vec3 v2 = vertex[2].position - vertex[1].position;
        vec3 flatFaceNormal = normalize(cross(v1, v2));

        // Update outputs to fragment shader
        for (int i=0; i<gl_in.length(); i++)
        {
            gl_Position = gl_in[i].gl_Position;

            vertexout.position = gl_in[i].gl_Position.xyz;
            vertexout.normal = flatFaceNormal;
            vertexout.color = vertex[i].color;
            vertexout.tcoord = vertex[i].tcoord;
            EmitVertex();
        }
        EndPrimitive();
    }
    else
    {
        // Update outputs to fragment shader
        for (int i=0; i<gl_in.length(); i++)
        {
            gl_Position = gl_in[i].gl_Position;

            vertexout.position = gl_in[i].gl_Position.xyz;
            vertexout.normal = vertex[i].normal;
            vertexout.color = vertex[i].color;
            vertexout.tcoord = vertex[i].tcoord;
            EmitVertex();
        }
        EndPrimitive();
    }
}