#version 440 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec3 texcoord;

out vec4 Color;
out vec2 Texcoord;

uniform mat4 trans;

void main()
{	
    gl_Position = trans * vec4(position, 1.0);
    Color = color;
}
