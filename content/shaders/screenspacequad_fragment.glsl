#version 330

in vec2 TexCoords;

uniform sampler2D quadTexture;
uniform float uOpacity;

out vec4 FragColor; 

void main()
{
    FragColor = texture(quadTexture, TexCoords);
    FragColor.a *= uOpacity;
}