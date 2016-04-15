#ifndef GL_UTILS_HPP__
#define GL_UTILS_HPP__

#include <string>
#include <istream>
#include <glad/glad.h>
#include <iostream>
#include <iterator>
#include <fstream>
#include <tuple>
#include <utility>
#include "PerlinNoise.hpp"

float rnd(float max);
float rnd();
float clamp(float x);

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
	ShaderSource(std::string filename);
	ShaderSource(const ShaderSource&) = delete;
	
	const GLchar** source() const;
	GLuint compile(GLenum type);
};

class ShaderProgram
{
public:
	ShaderSource vertexShaderSource_;
	ShaderSource fragmentShaderSource_;

	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint shaderProgram;

	ShaderProgram(std::string vertex_file, std::string fragment_file_);
	ShaderProgram(const ShaderProgram&) = delete;
	~ShaderProgram();

	void use();
	void setupAttributes();
};


struct color
{
	float r, g, b, a;

	color(): r(0), g(0), b(0), a(0) {}
	color(float r, float g, float b) : r(r), g(g), b(b), a(1) {}
	color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

	color mut(float d) {
		return {r + d, g + d, b + d, a};
	}
};

inline void push_color(std::vector<float>& vbo, color c) {
	vbo.push_back(c.r);
	vbo.push_back(c.g);
	vbo.push_back(c.b);
	vbo.push_back(c.a);
}

inline void push_color(std::vector<float>& v, float r, float g, float b, float a) {
	v.push_back(r);
	v.push_back(g);
	v.push_back(b);
	v.push_back(a);
}

#endif

