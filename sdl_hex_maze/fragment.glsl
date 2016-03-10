#version 150 core

out vec4 outColor;
in vec3 Color;
uniform vec3 triangleColor;

void main()
{
    outColor = vec4(Color, 1.0);
}