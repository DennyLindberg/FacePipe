#version 330

in vec4 TCoord;
in vec3 position;

layout(location = 0) out vec4 color;
uniform float gridSpacing;
uniform float opacity;
    
void main() 
{
    // Antialiased grid, slightly modified
    // http://madebyevan.com/shaders/grid/

    float gridScaling = 0.5f/gridSpacing;
    vec2 lineCoords = position.xy * gridScaling;
    vec2 grid = abs(fract(lineCoords-0.5)-0.5) / fwidth(lineCoords);
    float lineMask = min(1.0, min(grid.x, grid.y));

    color = vec4(lineMask, lineMask, lineMask, lineMask*opacity);
}