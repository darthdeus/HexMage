#ifndef GL_UTILS_HPP__
#define GL_UTILS_HPP__

#include <string>
#include <istream>
#include <glad/glad.h>
#include <iostream>
#include <iterator>
#include <fstream>
#include "PerlinNoise.hpp"

class ShaderSource {
	std::string filename_;
	std::string contents_;
	const char* c_str_;
	const GLchar** source_;

public:
	ShaderSource(std::string filename) : filename_(filename) {
		std::ifstream is{ filename };

		contents_ = { std::istreambuf_iterator<char>(is),
			std::istreambuf_iterator<char>() };

		c_str_ = contents_.c_str();
		source_ = &c_str_;
	}

	const GLchar** source() { return source_; }

	GLuint compile(GLenum type) {
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
};

class ShaderProgram
{
public:
	ShaderSource vertexShaderSource_;
	ShaderSource fragmentShaderSource_;

	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint shaderProgram;

	ShaderProgram(std::string vertex_file, std::string fragment_file_): vertexShaderSource_{vertex_file}, fragmentShaderSource_{fragment_file_}
	{
		vertexShader = vertexShaderSource_.compile(GL_VERTEX_SHADER);
		fragmentShader = fragmentShaderSource_.compile(GL_FRAGMENT_SHADER);

		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);	

		compileAndUse();
	}

	void compileAndUse()
	{
		glLinkProgram(shaderProgram);
		glUseProgram(shaderProgram);
	}
};

#endif
