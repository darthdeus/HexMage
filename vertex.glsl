#version 440 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in float useTexture;

out vec4 Color;
out vec2 Texcoord;
out float UseTexture;

layout (location = 0) uniform mat4 trans;

void main()
{	
    gl_Position = trans * vec4(position, 1.0);
    Color = color;
	Texcoord = texcoord;
	UseTexture = useTexture;
}
