#version 330

in vec2 TexCoords;

uniform sampler2D quadTexture;
uniform float uOpacity;
uniform bool uFlipY;

out vec4 FragColor; 

void main()
{
    if (uFlipY)
        FragColor = texture(quadTexture, vec2(TexCoords.x, 1.0f-TexCoords.y));
    else
        FragColor = texture(quadTexture, TexCoords);

    FragColor.a *= uOpacity;
}