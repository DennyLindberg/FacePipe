#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 6) out;

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

uniform float screenRatio;
uniform float size;

void Emit(vec3 pos, float w)
{
    vertexout.normal = vertex[0].normal;
    vertexout.color = vertex[0].color;
    vertexout.tcoord = vertex[0].tcoord;

    gl_Position = vec4(pos, w); 
    vertexout.position = gl_Position.xyz; 
    EmitVertex();
}

void main()
{
    vec3 center = gl_in[0].gl_Position.xyz;

    float w = gl_in[0].gl_Position.w;
    float x = size;
    float y = size;

    Emit(center + vec3(-x, y, 0.0f), w);
    Emit(center + vec3(-x, -y, 0.0f), w);
    Emit(center + vec3(x, -y, 0.0f), w);

    Emit(center + vec3(x, -y, 0.0f), w);
    Emit(center + vec3(x, y, 0.0f), w);
    Emit(center + vec3(-x, y, 0.0f), w);

    EndPrimitive();
}