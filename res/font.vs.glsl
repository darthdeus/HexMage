#version 440 core

layout(location = 0) in vec4 data;

out vec4 color;

layout(location = 0) uniform mat4 trans;
layout(location = 1) uniform mat4 projection;

void main() {
	gl_Position = projection * trans * vec4(data.xy, 0.0, 1.0);
	color = data.w;
}
