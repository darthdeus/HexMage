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

	ShaderSource(const ShaderSource&) = delete;

	const GLchar** source() { return source_; }

	GLuint compile(GLenum type) {
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

		glLinkProgram(shaderProgram);
		use();		
	}

	ShaderProgram(const ShaderProgram&) = delete;

	~ShaderProgram()
	{
		glDeleteProgram(shaderProgram);
		glDeleteShader(fragmentShader);
		glDeleteShader(vertexShader);
	}

	void use()
	{
		glUseProgram(shaderProgram);
	}

	void setupAttributes()
	{
		GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
		glEnableVertexAttribArray(posAttrib);
		glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);

		GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
		glEnableVertexAttribArray(colAttrib);
		glVertexAttribPointer(colAttrib, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
		glBindFragDataLocation(shaderProgram, 0, "outColor");
	}
};


float rnd(float max)
{
	return (static_cast<float>(rand()) / RAND_MAX) * max;
}

struct color
{
	float r, g, b, a;
  color(): r(0), g(0), b(0), a(0) {}
	color(float r, float g, float b) : r(r), g(g), b(r), a(1) {}
	color(float r, float g, float b, float a) : r(r), g(g), b(r), a(a) {}

	color mut(float d)
	{
		return{ r + d, g + d, b + d, a };
	}
};

void push_color(std::vector<float>& vbo, color c)
{
	vbo.push_back(c.r);
	vbo.push_back(c.g);
	vbo.push_back(c.b);
	vbo.push_back(c.a);
}

void push_color(std::vector<float>& v, float r, float g, float b, float a)
{
	v.push_back(r);
	v.push_back(g);
	v.push_back(b);
	v.push_back(a);
}

// TODO - test this
struct VBO
{
	GLuint buf;
	std::vector<float> vbo;
	ShaderProgram& program;

	VBO(ShaderProgram& program) : program{ program } {
		glGenBuffers(1, &buf);
		glBindBuffer(GL_ARRAY_BUFFER, buf);
		program.setupAttributes();
	}
	VBO(const VBO&) = delete;
	~VBO() { glDeleteBuffers(1, &buf); }

	void bufferData()
	{
		glBufferData(GL_ARRAY_BUFFER,
			sizeof(float) * vbo.size(),
			vbo.data(), GL_STATIC_DRAW);
	}

	void push_back(float x) { vbo.push_back(x); }
	void draw(GLenum type)
	{
		/*std::cout << "vbo[" << vbo.size() << "]\t";
		for (int i = 0; i < vbo.size(); i += 6)
		{
			std::cout << vbo[i] << "," << vbo[i + 1] << "  ";
		}
		std::cout << std::endl;*/

		bufferData();
		glDrawArrays(type, 0, vbo.size() / 6);
	}

	void push_vertex(float x, float y, color c)
	{
		vbo.push_back(x);
		vbo.push_back(y);
		push_color(vbo, c.r, c.g, c.b, c.a);
	}
};

#endif
