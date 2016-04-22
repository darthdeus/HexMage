#ifndef GL_UTILS_HPP__
#define GL_UTILS_HPP__

#include <string>
#include <istream>
#include <glad/glad.h>
#include <tuple>
#include <utility>
#include <vector>
#include <glm/glm.hpp>

enum class HexType
{
	Empty = 0,
	Wall,
	Player
};

struct Coord;
struct Cube;

struct Cube
{
	int x;
	int y;
	int z;

	Cube() : x(0), y(0), z(0) {}
	Cube(const Coord& axial);
	Cube(int x, int y, int z) : x(x), y(y), z(z) {}

	operator Coord() const;
	Cube abs() const;
	int min() const;
	int max() const;
};

struct Coord
{
	int x;
	int y;

	Coord() : x(0), y(0) {}
	Coord(const Cube& cube);
	Coord(int x, int y) : x(x), y(y) {}

	operator Cube() const;
	Coord abs() const;
	int min() const;
	int max() const;

	int distance() const;
};

Coord operator+(const Coord& lhs, const Coord& rhs);
Coord operator-(const Coord& lhs, const Coord& rhs);
bool operator==(const Coord& lhs, const Coord& rhs);
inline bool operator!=(const Coord& lhs, const Coord& rhs) { return !(lhs == rhs); }
std::ostream& operator<<(std::ostream& os, const Coord& c);

constexpr int ABILITY_COUNT = 6;

struct Position
{
	float x;
	float y;

	Position() : x(INFINITY), y(INFINITY) {}
	Position(const glm::vec2& v) : x(v.x), y(v.y) {}
	Position(float x, float y) : x(x), y(y) {}

	float distance() const;
	Position operator-() const;
	operator glm::vec2() const;

	Position abs() const;
	float min() const;
	float max() const;
	Position& operator+=(const Position& position);
	Position& operator-=(const Position& position);
};

Position operator+(const Position& lhs, const Position& rhs);
Position operator-(const Position& lhs, const Position& rhs);
bool operator==(const Position& lhs, const Position& rhs);
std::ostream& operator<<(std::ostream& os, const Position& p);

Position mouse2gl(int x, int y);

// TODO - update this to a proper container
template <typename T>
struct Matrix
{
	std::size_t m;
	std::size_t n;
	std::vector<T> vs;

	Matrix(std::size_t m, std::size_t n) : m(m), n(n), vs(m * n) {}

	// Create a matrix that can contain data for a hex with radius `size`
	Matrix(std::size_t size) : Matrix(size * 2 + 1, size * 2 + 1) {}

	T& operator()(std::size_t i, std::size_t j) {
		return vs[n * i + j];
	}

	T& operator()(const Coord& c) {
		return vs[n * c.y + c.x];
	}

private:
}; /* column-major/opengl: vs[i + m * j], row-major/c++: vs[n * i + j] */


struct Color
{
	float r, g, b, a;

	Color() : r(0), g(0), b(0), a(0) {}
	Color(float r, float g, float b) : r(r), g(g), b(b), a(1) {}
	Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

	Color mut(float d) const {
		return{ r + d, g + d, b + d, a };
	}
};


Color color_for_type(HexType type);
float rad_for_hex(int i);
void push_vertex(std::vector<float>& vbo, float x, float y, Color c);
void hex_at(std::vector<float>& vertices, Position pos, float r, Color c);

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

inline void push_color(std::vector<float>& vbo, Color c) {
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

namespace gl
{
	class Shader
	{
	public:
		GLuint program;
		
		Shader(const GLchar* vertexPath, const GLchar* fragmentPath);
		~Shader();

		void use();
	};
}



#endif

