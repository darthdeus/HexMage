#include "gl_utils.hpp"

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