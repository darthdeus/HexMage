#version 150 core

out vec4 outColor;
in vec4 Color;
uniform vec3 triangleColor;

void main()
{
    outColor = vec4(Color);
}