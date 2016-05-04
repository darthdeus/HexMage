#ifndef GL_UTILS_HPP__
#define GL_UTILS_HPP__

#include <iostream>
#include <string>
#include <istream>
#include <functional>
#include <tuple>
#include <utility>
#include <vector>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL/SDL_hints.h>
#include <lodepng.h>

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

inline bool operator!=(const Coord& lhs, const Coord& rhs)
{
	return !(lhs == rhs);
}

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

	operator glm::vec2()
	{
		return {x, y};
	}
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

	T& operator()(std::size_t i, std::size_t j)
	{
		return vs[n * i + j];
	}

	T& operator()(const Coord& c)
	{
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

	Color mut(float d) const
	{
		return {r + d, g + d, b + d, a};
	}

	operator glm::vec4() const
	{
		return {r, g, b, a};
	}
};


Color color_for_type(HexType type);
float rad_for_hex(int i);

namespace gl
{
	class Camera
	{
		glm::mat4 projection_{1};
		glm::mat4 zoom_{1};
		glm::mat4 mov_{1};
		Position current_scroll_{0, 0};
		Position translate_{0, 0};
		float zoom_level_ = 0.7f;

		const float scroll_offset = 0.05f;
	public:

		void update_camera();
		void update_and_load_camera();

		glm::mat4 projection() const;
		float* value_ptr();

		void keydown(Sint32 key);
		void keyup(Sint32 key);
		void scroll(Sint32 direction);
	};

	class VAO
	{
	public:
		GLuint id;
		VAO() { glGenVertexArrays(1, &id); bind(); }
		~VAO() { glDeleteVertexArrays(1, &id); }

		VAO(const VAO& other) = delete;
		VAO(VAO&& other) = delete;
		VAO& operator=(const VAO& other) = delete;
		VAO& operator=(VAO&& other) = delete;

		void bind() const { glBindVertexArray(id); }
		void unbind() const { glBindVertexArray(0); }
	};

	class VBO
	{
	public:
		GLuint id;

		VBO() { glGenBuffers(1, &id); bind(); }
		~VBO() { glDeleteBuffers(1, &id); }

		VBO(const VBO& other) = delete;
		VBO(VBO&& other) = delete;
		VBO& operator=(const VBO& other) = delete;
		VBO& operator=(VBO&& other) = delete;

		void bind() const { glBindBuffer(GL_ARRAY_BUFFER, id); }
		void unbind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }
	};

	class Texture2D
	{
	public:
		GLuint id, width, height, internal_format, image_format;
		GLuint wrap_s, wrap_t, filter_min, filter_mag;

		bool invalid = false;

		Texture2D():
			width(0), height(0),
			internal_format(GL_RGB), image_format(GL_RGB),
			wrap_s(GL_REPEAT), wrap_t(GL_REPEAT),
			filter_min(GL_LINEAR), filter_mag(GL_LINEAR) {
			glGenTextures(1, &id);
			std::cout << "Texture2D() generated" << std::endl;
		}

		Texture2D(const Texture2D&) = delete;
		Texture2D& operator=(const Texture2D&) = delete;

		~Texture2D() {
			glDeleteTextures(1, &id);
			std::cout << "~Texture2D()" << std::endl;
		}

		void load_png(const std::string& filename) {
			std::vector<unsigned char> data;
			lodepng::decode(data, width, height, filename);
			load(width, height, data.data());
		}
		
		void load(GLuint width, GLuint height, unsigned char* data) {
			this->width = width;
			this->height = height;

			glBindTexture(GL_TEXTURE_2D, id);	
			glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, image_format, GL_UNSIGNED_BYTE, data);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_min);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_mag);
			
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		void bind() const {
			glBindTexture(GL_TEXTURE_2D, id);
		}
	};

	class Shader
	{
	public:
		GLuint program;

		Shader(const GLchar* vertexPath, const GLchar* fragmentPath);

		Shader(const Shader& other) = delete;
		Shader(Shader&& other) = delete;
		Shader& operator=(const Shader& other) = delete;
		Shader& operator=(Shader&& other) = delete;

		~Shader();

		void set(const GLchar* name, int value);
		void set(const GLchar* name, float value);
		void set(const GLchar* name, const glm::vec2& v);
		void set(const GLchar* name, const glm::vec3& v);
		void set(const GLchar* name, const glm::vec4& v);
		void set(const GLchar* name, const glm::mat4& matrix);

		void use();
	};

	class SpriteRenderer
	{
	public:
		explicit SpriteRenderer(Shader& shader): shader(shader) {
			initialize();
		}

		SpriteRenderer(const SpriteRenderer& other) = delete;
		SpriteRenderer(SpriteRenderer&& other) = delete;
		SpriteRenderer& operator=(const SpriteRenderer& other) = delete;
		SpriteRenderer& operator=(SpriteRenderer&& other) = delete;

		~SpriteRenderer() = default;

		void draw_sprite(Texture2D& texture, glm::vec2 pos, glm::vec2 size = glm::vec2(10, 10), glm::vec3 color = glm::vec3(1.0f));
	private:
		Shader& shader;

		VAO vao;
		VBO vbo;
		void initialize();
	};

	struct ColorTex
	{
		glm::vec4 color;
		glm::vec2 tex;
		GLfloat useTexture;

		ColorTex(float x, float y, float z, float w)
			: color(x,y,z,w), tex(0, 0), useTexture(false) {}

		ColorTex(float s, float t)
			: color(1,1,1,1), tex(s, t), useTexture(true) {}

		ColorTex(const glm::vec4& color)
			: ColorTex(color, {0, 0}, false) {}

		ColorTex(const glm::vec2& tex)
			: ColorTex({1,1,1,1}, tex, true) {}

		ColorTex(const glm::vec4& color, const glm::vec2& tex, GLfloat useTexture)
			: color(color),
			  tex(tex),
			  useTexture(useTexture) {}
	};

	class Vertex
	{
	public:
		glm::vec3 position;
		glm::vec4 color;
		glm::vec2 texCoord;
		GLfloat useTexture;

		Vertex(glm::vec3 position):
			Vertex(std::move(position), {1,1,1,1}, {0,0}, false) {}

		Vertex(glm::vec3 position, ColorTex ct) :
			Vertex(std::move(position), std::move(ct.color), std::move(ct.tex), ct.useTexture) {}

		Vertex(glm::vec3 position, glm::vec4 color, glm::vec2 tex_coord, GLfloat useTexture):
			position{std::move(position)},
			color{std::move(color)},
			texCoord{std::move(tex_coord)},
			useTexture(useTexture) {}

		static void setup_attributes() {
			GLsizei stride = sizeof(gl::Vertex);
			glVertexAttribPointer(0, sizeof(position) / sizeof(float), GL_FLOAT, GL_FALSE, stride, (GLvoid*)offsetof(Vertex, position));
			glVertexAttribPointer(1, sizeof(color) / sizeof(float), GL_FLOAT, GL_FALSE, stride, (GLvoid*)offsetof(Vertex, color));
			glVertexAttribPointer(2, sizeof(texCoord) / sizeof(float), GL_FLOAT, GL_FALSE, stride, (GLvoid*)offsetof(Vertex, texCoord));
			glVertexAttribPointer(3, sizeof(useTexture) / sizeof(GLfloat), GL_FLOAT, GL_FALSE, stride, (GLvoid*)offsetof(Vertex, useTexture));
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glEnableVertexAttribArray(3);
		}
	};

	class Batch
	{
	public:
		using v2 = glm::vec2;
		using v3 = glm::vec3;
		using v4 = glm::vec4;

		std::vector<Vertex> vertices;

		void clear();
		void push_back(Vertex v);
		void draw_arrays();

		// TODO - cleanup these overloads
		void push_triangle(v2 p1, v2 p2, v2 p3, float z, ColorTex ct);
		void push_quad(v2 p1, v2 p2, v2 p3, v2 p4, float z, ColorTex ct);

		void push_quad(v2 center, float width, float height, float z, ColorTex ct);
		void push_quad_bot_left(v2 bot_left, float width, float height, float z, ColorTex ct);

		void push_hex(v2 position, v3 color, float r);
		void push_hex(v2 position, v4 color, float r);
		void push_hex(v3 position, v4 color, float r);
	};

	//class Scene
	//{
	//public:
	//	Batch batch(GLenum mode) {
	//		
	//	}
	//};
}


#endif
