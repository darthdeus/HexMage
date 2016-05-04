#ifndef GL_UTILS_HPP__
#define GL_UTILS_HPP__

#include <functional>
#include <tuple>
#include <vector>
#include <map>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL/SDL_hints.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <format.h>

float rad_for_hex(int i);

namespace gl
{
	class Camera
	{
		glm::mat4 projection_{1};
		glm::mat4 zoom_{1};
		glm::mat4 mov_{1};
		glm::vec2 current_scroll_{0, 0};
		glm::vec2 translate_{0, 0};
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

		Texture2D();
		~Texture2D();

		Texture2D(const Texture2D&) = delete;
		Texture2D(Texture2D&&) = delete;
		Texture2D& operator=(const Texture2D&) = delete;
		Texture2D& operator=(Texture2D&&) = delete;

		void load_png(const std::string& filename);
		void load(GLuint width, GLuint height, unsigned char* data);
		void bind() const;
	};

	class Shader
	{
	public:
		GLuint program;

		explicit Shader(std::string name);
		Shader(std::string vertex, std::string fragment);

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
		explicit SpriteRenderer(Shader& shader);

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

	struct Character
	{
		GLuint tex;
		glm::ivec2 size;
		glm::ivec2 bearing;
		GLuint advance;
	};

	class FontAtlas
	{
		std::map<GLchar, Character> characters;

		void init();
	};
}


#endif
