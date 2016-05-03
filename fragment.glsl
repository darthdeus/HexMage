#version 440 core

in vec4 Color;
in vec2 Texcoord;
in float UseTexture;

uniform sampler2D t;
out vec4 color;

void main()
{
	//color = Color * texture(t, Texcoord.st);
	 if (UseTexture > 0.5) {
	 	color = Color * texture(t, Texcoord.st);
	 } else {
	 	color = Color;
	 }
}