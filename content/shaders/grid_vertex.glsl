#version 330

layout(location = 0) in vec3 vertexPosition;
layout(location = 3) in vec4 vertexTCoord;

uniform mat4 mvp;
uniform float size;
uniform bool uFlipY;

out vec4 TCoord;
out vec3 position;

void main()
{

    gl_Position = mvp * vec4(vertexPosition*size, 1.0f);
    position = vertexPosition*size;

    TCoord = vertexTCoord;
    if (uFlipY)
        TCoord.y = 1.0f-vertexTCoord.y;
}