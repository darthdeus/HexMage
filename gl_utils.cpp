#define _USE_MATH_DEFINES
#include <math.h>
#include "gl_utils.hpp"
// TODO - remove this dependency
#include "model.hpp"

float rnd(float max) {
	return (static_cast<float>(rand()) / RAND_MAX) * max;
}

float rnd() {
	return rnd(1.0f);
}

float clamp(float x) {
	if (x < 0)
		return 0;
	if (x > 1)
		return 1;
	return x;
}

ShaderSource::ShaderSource(std::string filename) : filename_(filename) {
	std::ifstream is{ filename };

	contents_ = { std::istreambuf_iterator<char>(is),
		std::istreambuf_iterator<char>() };

	c_str_ = contents_.c_str();
	source_ = &c_str_;
}

const GLchar** ShaderSource::source() const {
	return source_;
}

GLuint ShaderSource::compile(GLenum type) {
	// TODO - deallocate resources
	GLuint shaderID = glCreateShader(type);
	glShaderSource(shaderID, 1, source(), nullptr);
	glCompileShader(shaderID);

	GLint status;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		std::cerr << "Shader from " << filename_ << " failed to compile"
			<< std::endl;

		char buffer[512];
		glGetShaderInfoLog(shaderID, 512, nullptr, buffer);
		std::cerr << buffer << std::endl;
	}

	return shaderID;
}

ShaderProgram::ShaderProgram(std::string vertex_file, std::string fragment_file_) : vertexShaderSource_{ vertex_file }, fragmentShaderSource_{ fragment_file_ } {
	vertexShader = vertexShaderSource_.compile(GL_VERTEX_SHADER);
	fragmentShader = fragmentShaderSource_.compile(GL_FRAGMENT_SHADER);

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	glLinkProgram(shaderProgram);
	use();
}


ShaderProgram::~ShaderProgram() {
	glDeleteProgram(shaderProgram);
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
}

void ShaderProgram::use() {
	glUseProgram(shaderProgram);
}

void ShaderProgram::setupAttributes() {
	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);

	GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
	glEnableVertexAttribArray(colAttrib);
	glVertexAttribPointer(colAttrib, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glBindFragDataLocation(shaderProgram, 0, "outColor");
}

color color_for_type(model::HexType type) {
	switch (type) {
	case model::HexType::Empty:
		return{ 0.4f, 0.2f, 0.4f };
	case model::HexType::Wall:
		return{ 0.1f, 0.03f, 0.1f };
	case model::HexType::Player:
		return{ 0.7f, 0.4f, 0.7f };
	default:
		throw "invalid hex type";
	}
}

float rad_for_hex(int i) {
	float angle_deg = static_cast<float>(60 * i + 30);
	return static_cast<float>(M_PI) / 180 * angle_deg;
}

void push_vertex(std::vector<float>& vbo, float x, float y, color c) {
	vbo.push_back(x);
	vbo.push_back(y);
	push_color(vbo, c.r, c.g, c.b, c.a);
}

void hex_at(std::vector<float>& vertices, model::Position pos, float r, color c) {
	float ri;
	int rot = 0; // 1;
	for (int i = rot; i < 7 + rot; i++) {
		push_vertex(vertices, pos.x, pos.y, c);

		ri = rad_for_hex(i - 1);
		c = c.mut(0.015f);
		push_vertex(vertices, pos.x + r * cos(ri), pos.y + r * sin(ri), c);

		ri = rad_for_hex(i);
		c = c.mut(0.015f);
		push_vertex(vertices, pos.x + r * cos(ri), pos.y + r * sin(ri), c);
	}
}
