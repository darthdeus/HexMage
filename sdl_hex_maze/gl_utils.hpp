#ifndef GL_UTILS_HPP__
#define GL_UTILS_HPP__

#include <string>
#include <istream>
#include <glad/glad.h>
#include <iostream>
#include <iterator>
#include <fstream>
#include "PerlinNoise.hpp"

inline float rnd(float max) {
	return (static_cast<float>(rand()) / RAND_MAX) * max;
}

inline float rnd() {
	return rnd(1.0f);
}

inline float clamp(float x) {
	if (x < 0)
		return 0;
	if (x > 1)
		return 1;
	return x;
}

#define RZ(x, m) (clamp((x)+rnd(m) - (m) / 2))

struct hash_int_triple
{
	std::size_t operator()(const std::tuple<int, int, int>& tup) const {
		using namespace std;

		return hash<int>()(get<0>(tup)) ^ (hash<int>()(get<1>(tup)) << 1) ^
				(hash<int>()(get<2>(tup)) << 1);
	}
};

struct hash_int_pair
{
	std::size_t operator()(const std::pair<int, int>& tup) const {
		using namespace std;

		return hash<int>()(tup.first) ^ (hash<int>()(tup.second) << 1);
	}
};

template <typename T>
struct matrix
{
	unsigned m;
	unsigned n;
	std::vector<T> vs;

	matrix(unsigned m, unsigned n) : m(m), n(n), vs(m * n) {}

	T& operator ()(unsigned i, unsigned j) {
		return vs[n * i + j];
	}

private:
}; /* column-major/opengl: vs[i + m * j], row-major/c++: vs[n * i + j] */

// TODO - do this in a less insane way
class ShaderSource
{
	std::string filename_;
	std::string contents_;
	const char* c_str_;
	const GLchar** source_;

public:
	ShaderSource(std::string filename) : filename_(filename) {
		std::ifstream is{filename};

		contents_ = {std::istreambuf_iterator<char>(is),
			std::istreambuf_iterator<char>()};

		c_str_ = contents_.c_str();
		source_ = &c_str_;
	}

	ShaderSource(const ShaderSource&) = delete;

	const GLchar** source() const {
		return source_;
	}

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

	ShaderProgram(std::string vertex_file, std::string fragment_file_): vertexShaderSource_{vertex_file}, fragmentShaderSource_{fragment_file_} {
		vertexShader = vertexShaderSource_.compile(GL_VERTEX_SHADER);
		fragmentShader = fragmentShaderSource_.compile(GL_FRAGMENT_SHADER);

		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);

		glLinkProgram(shaderProgram);
		use();
	}

	ShaderProgram(const ShaderProgram&) = delete;

	~ShaderProgram() {
		glDeleteProgram(shaderProgram);
		glDeleteShader(fragmentShader);
		glDeleteShader(vertexShader);
	}

	void use() {
		glUseProgram(shaderProgram);
	}

	void setupAttributes() {
		GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
		glEnableVertexAttribArray(posAttrib);
		glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);

		GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
		glEnableVertexAttribArray(colAttrib);
		glVertexAttribPointer(colAttrib, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
		glBindFragDataLocation(shaderProgram, 0, "outColor");
	}
};


struct color
{
	float r, g, b, a;

	color(): r(0), g(0), b(0), a(0) {}

	color(float r, float g, float b) : r(r), g(g), b(r), a(1) {}

	color(float r, float g, float b, float a) : r(r), g(g), b(r), a(a) {}

	color mut(float d) {
		return {r + d, g + d, b + d, a};
	}
};

void push_color(std::vector<float>& vbo, color c) {
	vbo.push_back(c.r);
	vbo.push_back(c.g);
	vbo.push_back(c.b);
	vbo.push_back(c.a);
}

void push_color(std::vector<float>& v, float r, float g, float b, float a) {
	v.push_back(r);
	v.push_back(g);
	v.push_back(b);
	v.push_back(a);
}

#endif

