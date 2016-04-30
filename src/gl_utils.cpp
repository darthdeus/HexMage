#define _USE_MATH_DEFINES
#include <math.h>
#include <fstream>
#include <iterator>
#include <sstream>
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

GLuint load_and_compile_shader_(const GLchar* path, GLenum shaderType) {
	using namespace std;

	ifstream file(path);

	stringstream str;
	str << file.rdbuf();

	string code = str.str();
	//cout << code << endl << endl;
	const GLchar* code_c = code.c_str();

	GLint success;
	GLchar errorLog[512];

	GLuint shaderId = glCreateShader(shaderType);
	glShaderSource(shaderId, 1, &code_c, nullptr);
	glCompileShader(shaderId);

	// Check for compile errors
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shaderId, 512, nullptr, errorLog);
		cerr << "ERROR: shader " << path << " failed to compile: " << errorLog << endl;
		throw "Shader failed to compile";
	}

	return shaderId;
}

namespace gl
{
	void Camera::update_camera() {
		translate_ += current_scroll_;

		mov_ = glm::translate(glm::mat4(1.0), glm::vec3(static_cast<glm::vec2>(translate_), 0));
		zoom_ = glm::scale(glm::mat4(1.0f), glm::vec3(zoom_level_));
		projection_ = zoom_ * mov_;
	}

	glm::mat4 Camera::projection() const {
		return projection_;
	}

	float* Camera::value_ptr() {
		return glm::value_ptr(projection_);
	}

	void Camera::keydown(Sint32 key) {
		switch (key) {
			case 'w':
				current_scroll_.y = -scroll_offset;
				break;
			case 's':
				current_scroll_.y = scroll_offset;
				break;
			case 'a':
				current_scroll_.x = scroll_offset;
				break;
			case 'd':
				current_scroll_.x = -scroll_offset;
				break;
		}
	}

	void Camera::keyup(Sint32 key) {
		switch (key) {
			case 'w':
			case 's':
				current_scroll_.y = 0;
				break;

			case 'a':
			case 'd':
				current_scroll_.x = 0;
				break;
		}
	}

	void Camera::scroll(Sint32 direction) {
		zoom_level_ += 0.07f * direction;
	}

	Shader::Shader(const GLchar* vertexPath, const GLchar* fragmentPath) {
		using namespace std;

		GLuint vertex = load_and_compile_shader_(vertexPath, GL_VERTEX_SHADER);
		GLuint fragment = load_and_compile_shader_(fragmentPath, GL_FRAGMENT_SHADER);

		program = glCreateProgram();
		glAttachShader(program, vertex);
		glAttachShader(program, fragment);
		glLinkProgram(program);

		GLint success;
		GLchar errorLog[512];
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(program, 512, nullptr, errorLog);
			cerr << "ERROR: program link fail: " << errorLog << endl;
			throw "glLinkProgram failed";
		}

		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	Shader::~Shader() { glDeleteProgram(program); }

	void Shader::use() { glUseProgram(program); }

	void Batch::clear() {
		vertices.clear();
	}

	void Batch::push_back(Vertex v) {
		vertices.push_back(std::move(v));
	}

	void Batch::draw_arrays() {
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices.size());
	}

	float rad_for_hex(int i) {
		float angle_deg = static_cast<float>(60 * i + 30);
		return static_cast<float>(M_PI) / 180 * angle_deg;
	}

	void Batch::push_hex(glm::vec2 position, glm::vec3 color, float r) {
		push_hex(glm::vec3(position, 0), glm::vec4(color, 1), r);
	}

	void Batch::push_hex(glm::vec2 position, glm::vec4 color, float r) {
		push_hex(glm::vec3(position, 0), color, r);
	}

	void Batch::push_hex(glm::vec3 position, glm::vec4 color, float r) {
		float ri;
		int rot = 0; // 1;

		glm::vec3 c = {color.x, color.y, color.z};

		for (int i = rot; i < 6 + rot; i++) {
			push_back({position, {c.x, c.y, c.z, color.w}});
			ri = rad_for_hex(i - 1);
			c += 0.015f;

			push_back({
				{position.x + r * cos(ri), position.y + r * sin(ri), position.z},
				{c.x, c.y, c.z, color.w}
			});

			ri = rad_for_hex(i);
			c += 0.015f;

			push_back({
				{position.x + r * cos(ri), position.y + r * sin(ri), position.z},
				{c.x, c.y, c.z, color.w}
			});
		}
	}
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
	vbo.push_back(0.0f);
	push_color(vbo, c.r, c.g, c.b, c.a);
}

void hex_at(std::vector<float>& vertices, Position pos, float r, Color c) {
	float ri;
	int rot = 0; // 1;

	for (int i = rot; i < 6 + rot; i++) {
		push_vertex(vertices, pos.x, pos.y, c);

		ri = rad_for_hex(i - 1);
		c = c.mut(0.015f);
		push_vertex(vertices, pos.x + r * cos(ri), pos.y + r * sin(ri), c);

		ri = rad_for_hex(i);
		c = c.mut(0.015f);
		push_vertex(vertices, pos.x + r * cos(ri), pos.y + r * sin(ri), c);
	}
}
