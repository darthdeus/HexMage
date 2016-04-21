#define _USE_MATH_DEFINES
#include <math.h>
#include <fstream>
#include <iterator>
#include <model.hpp>

Cube::Cube(const Coord& axial) : x(axial.x), y(-axial.x - axial.y), z(axial.y) {}
Cube::operator Coord() const { return{ *this }; }
Cube Cube::abs() const { return{ std::abs(x), std::abs(y), std::abs(z) }; }
int Cube::min() const { return std::min(x, std::min(y, z)); }
int Cube::max() const { return std::max(x, std::max(y, z)); }

Coord::Coord(const Cube& cube) : x(cube.x), y(cube.z) {}
Coord::operator Cube() const { return{ *this }; }
Coord Coord::abs() const { return{ std::abs(x), std::abs(y) }; }
int Coord::min() const { return std::min(x, y); }
int Coord::max() const { return std::max(x, y); }
int Coord::distance() const { return Cube(*this).abs().max(); }

Coord operator+(const Coord& lhs, const Coord& rhs) { return{ lhs.x + rhs.x, lhs.y + rhs.y }; }
Coord operator-(const Coord& lhs, const Coord& rhs) { return{ lhs.x - rhs.x, lhs.y - rhs.y }; }
bool operator==(const Coord& lhs, const Coord& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }

Position operator+(const Position& lhs, const Position& rhs) { return{ lhs.x + rhs.x, lhs.y + rhs.y }; }
Position operator-(const Position& lhs, const Position& rhs) { return{ lhs.x - rhs.x, lhs.y - rhs.y }; }
bool operator==(const Position& lhs, const Position& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }

std::ostream& operator<<(std::ostream& os, const Position& p) {
	return os << "(" << p.x << "," << p.y << ")";
}

float Position::distance() const { return x * x + y * y; }

Position Position::operator-() const { return{ -x, -y }; }
Position::operator glm::vec2() const { return{ x, y }; }

Position Position::abs() const { return{ std::abs(x), std::abs(y) }; }
float Position::min() const { return std::min(x, y); }
float Position::max() const { return std::max(x, y); }

Position& Position::operator+=(const Position& p) { x += p.x; y += p.y; return *this; }
Position& Position::operator-=(const Position& p) { x -= p.x; y -= p.y; return *this; }

std::ostream& operator<<(std::ostream& os, const Coord& c) {
	return os << "(" << c.x << "," << c.y << ")";
}

Position mouse2gl(int x, int y) {
	// TODO - figure out a place to put this
	constexpr int SCREEN_WIDTH = 800;
	constexpr int SCREEN_HEIGHT = 600;

	float rel_x = static_cast<float>(x) / SCREEN_WIDTH;
	float rel_y = static_cast<float>(y) / SCREEN_HEIGHT;

	rel_y = 2 * (1 - rel_y) - 1;
	rel_x = 2 * rel_x - 1;

	return{ rel_x, rel_y };
}

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

Color color_for_type(HexType type) {
	switch (type) {
	case HexType::Empty:
		return{ 0.4f, 0.2f, 0.4f };
	case HexType::Wall:
		return{ 0.1f, 0.03f, 0.1f };
	case HexType::Player:
		return{ 0.7f, 0.4f, 0.7f };
	default:
		throw "invalid hex type";
	}
}

float rad_for_hex(int i) {
	float angle_deg = static_cast<float>(60 * i + 30);
	return static_cast<float>(M_PI) / 180 * angle_deg;
}

void push_vertex(std::vector<float>& vbo, float x, float y, Color c) {
	vbo.push_back(x);
	vbo.push_back(y);
	push_color(vbo, c.r, c.g, c.b, c.a);
}

void hex_at(std::vector<float>& vertices, Position pos, float r, Color c) {
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
