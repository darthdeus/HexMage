#version 410 core

layout(location = 0) in vec4 data;

out vec2 tex;

uniform mat4 trans;
uniform mat4 projection;

void main() {
	gl_Position = projection * trans * vec4(data.xy, 0.0, 1.0);
	tex = data.zw;
}
