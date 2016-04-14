#version 150 core

out vec4 outColor;
in vec4 Color;

void main()
{
    outColor = vec4(Color);
}