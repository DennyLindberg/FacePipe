#version 330

layout(location = 0) in vec3 vertexPosition;
layout(location = 3) in vec4 vertexTCoord;
uniform mat4 mvp;
uniform float size;

out vec4 TCoord;
out vec3 position;

void main()
{
    gl_Position = mvp * vec4(vertexPosition*size, 1.0f);
    TCoord = vertexTCoord;
    position = vertexPosition*size;
}