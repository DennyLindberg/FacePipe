#version 330

layout(location = 0) in vec3 vertexPosition;
layout(location = 3) in vec4 vertexTCoord;

uniform vec2 uPos;
uniform vec2 uSize;
uniform bool uFlipY;

out vec2 TexCoords;

void main()
{

    // Scale based on center
    gl_Position.x = vertexPosition.x*uSize.x + uPos.x - 1.0f;
    gl_Position.y = vertexPosition.y*uSize.y - uPos.y + 1.0f;
    gl_Position.z = vertexPosition.z;
    gl_Position.w = 1.0f;

    TexCoords = vec2(vertexTCoord.x, vertexTCoord.y);
    if (uFlipY)
        TexCoords.y = 1.0f-vertexTCoord.y;
}