#version 330 core
layout (location = 0) in vec4 vertex;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 projection;

void main() {
	TexCoords = vertex.zw;
	//const vec2 vertices[6] = vec2[6](
	//vec2(0.25, -0.25),
	//vec2(-0.25, -0.25),
	//vec2(0.25, 0.25),
	//vec2(0.25, -0.25),
	//vec2(0.25, 0.25),
	//vec2(0, 0)
	//);
	const vec2 vertices[6] = vec2[6](
	vec2(25, -25),
	vec2(-25, -25),
	vec2(25, 25),
	vec2(25, -25),
	vec2(25, 25),
	vec2(0, 0)
	);
	 gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
	//gl_Position = vec4(vertex.xy, 0.0, 1.0);
	//gl_Position = projection * model * vec4(vertices[gl_VertexID].xy, 0.0, 1.0);
}