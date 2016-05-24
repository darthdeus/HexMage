#include <glad/glad.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <fstream>
#include <iterator>
#include <sstream>
#include <iostream>

#include <gl_utils.hpp>

#include <model.hpp>
#include <lodepng.h>

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

	void Camera::update_and_load_camera() {
		update_camera();
		glUniformMatrix4fv(0, 1, GL_FALSE, value_ptr());
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

	Texture2D::Texture2D():
		width(0), height(0),
		internal_format(GL_RGB), image_format(GL_RGB),
		wrap_s(GL_REPEAT), wrap_t(GL_REPEAT),
		filter_min(GL_LINEAR), filter_mag(GL_LINEAR) {
		glGenTextures(1, &id);
	}

	Texture2D::~Texture2D() {
		glDeleteTextures(1, &id);
	}

	void Texture2D::load_png(const std::string& filename) {
		std::vector<unsigned char> data;
		lodepng::decode(data, width, height, filename);
		load(width, height, data.data());
	}

	void Texture2D::load(GLuint width, GLuint height, unsigned char* data) {
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

	void Texture2D::bind() const {
		glBindTexture(GL_TEXTURE_2D, id);
	}

	Shader::Shader(std::string name): Shader(name + ".vs.glsl", name + ".fs.glsl") { }

	Shader::Shader(std::string vertexPath, std::string fragmentPath) {
		using namespace std;

		GLuint vertex = load_and_compile_shader_(vertexPath.c_str(), GL_VERTEX_SHADER);
		GLuint fragment = load_and_compile_shader_(fragmentPath.c_str(), GL_FRAGMENT_SHADER); 
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

	void Shader::set(const GLchar* name, int value) {
		use();
		glUniform1i(glGetUniformLocation(program, name), value);
	}

	void Shader::set(const GLchar* name, float value) {
		use();
		glUniform1f(glGetUniformLocation(program, name), value);
	}

	void Shader::set(const GLchar* name, const glm::vec2& v) {
		use();
		glUniform2f(glGetUniformLocation(program, name), v.x, v.y);
	}

	void Shader::set(const GLchar* name, const glm::vec3& v) {
		use();
		glUniform3f(glGetUniformLocation(program, name), v.x, v.y, v.z);
	}

	void Shader::set(const GLchar* name, const glm::vec4& v) {
		use();
		glUniform4f(glGetUniformLocation(program, name), v.x, v.y, v.z, v.w);
	}

	void Shader::set(const GLchar* name, const glm::mat4& matrix) {
		use();
		glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void Shader::use() { glUseProgram(program); }

	SpriteRenderer::SpriteRenderer(): SpriteRenderer("res/sprite") {}

	SpriteRenderer::SpriteRenderer(std::string shader_path): shader(shader_path) {
		GLfloat vertices[] = {
			0, 1, 0, 1,
			1, 0, 1, 0,
			0, 0, 0, 0,

			0, 1, 0, 1,
			1, 1, 1, 1,
			1, 0, 1, 0
		};

		vbo.bind();
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		vao.bind();
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);

		vbo.unbind();
		vao.unbind();
	}

	void SpriteRenderer::draw_sprite(Texture2D& texture, glm::vec2 pos, glm::vec2 size, glm::vec3 color)
	{
		texture.bind();
		shader.use();
		glm::mat4 model(1.0f);

		model = glm::translate(model, glm::vec3(pos, 0.0f));
		model = glm::scale(model, glm::vec3(size, 1.0f));

		shader.set("model", model);
		shader.set("spriteColor", {1,1,1});

		glActiveTexture(GL_TEXTURE0);
		texture.bind();

		vao.bind();
		glDrawArrays(GL_TRIANGLES, 0, 6);
		vao.unbind();
	}

	void SpriteRenderer::set_projection(const glm::mat4& projection)
	{
		shader.set("projection", projection);
	}

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

	void Batch::push_triangle(v2 p1, v2 p2, v2 p3, float z, ColorTex ct) {
		push_back({ v3(p1, z), ct.color, ct.tex, ct.useTexture });
		push_back({ v3(p2, z), ct.color, ct.tex, ct.useTexture });
		push_back({ v3(p3, z), ct.color, ct.tex, ct.useTexture });
	}

	void Batch::push_quad(v2 p1, v2 p2, v2 p3, v2 p4, float z, ColorTex ct) {
		push_triangle(p1, p2, p3, z, ct);
		push_triangle(p1, p3, p4, z, ct);
	}

	void Batch::push_quad(v2 center, float width, float height, float z, ColorTex ct) {
		push_quad(
			{center.x - width / 2, center.y - height / 2},
			{center.x + width / 2, center.y - height / 2},
			{center.x + width / 2, center.y + height / 2},
			{center.x - width / 2, center.y + height / 2},
			z, ct
		);
	}

	void Batch::push_quad_bot_left(v2 bot_left, float width, float height, float z, ColorTex ct) {
		push_quad(bot_left,
		          {bot_left.x + width, bot_left.y},
		          {bot_left.x + width, bot_left.y + height},
		          {bot_left.x, bot_left.y + height},
		          z, ct);
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

		glm::vec3 c = { color.x, color.y, color.z };

		for (int i = rot; i < 6 + rot; i++) {
			push_back({ position, {c.x, c.y, c.z, color.w} });
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

  // Color color_for_type(model::HexType type) {
  //   switch (type) {
  //     case HexType::Empty:
  //       return{ 0.4f, 0.2f, 0.4f };
  //     case HexType::Wall:
  //       return{ 0.1f, 0.03f, 0.1f };
  //     case HexType::Player:
  //       return{ 0.7f, 0.4f, 0.7f };
  //     default:
  //       return {1.0f, 1.0f, 1.0f};
  //       // throw "invalid hex type";
  //   }
  // }

	void FontAtlas::init(int size) {
		if (initialized_) return;

		vao.bind();
		vbo.bind();
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		vao.unbind();

		FT_Library ft;
		if (FT_Init_FreeType(&ft)) {
			fmt::print("ERROR::FREETYPE: Could not init FreeType\n");
		}

		FT_Face face;
		if (FT_New_Face(ft, "res/ProggyClean.ttf", 0, &face)) {
		//if (FT_New_Face(ft, "c:/windows/fonts/times.ttf", 0, &face)) {
			fmt::print("ERROR::FREETYPE: Failed to load font\n");
		}

		FT_Set_Pixel_Sizes(face, 0, size);
		if (FT_Load_Char(face, 'X', FT_LOAD_RENDER)) {
			fmt::printf("ERROR::FREETYPE: Failed to load Glyph\n");
		}

		for (GLubyte c = 0; c < 128; ++c) {
			if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
				fmt::printf("ERROR::FREETYPE: Failed to load Glyph '%c'\n", c);
				continue;
			}

			GLuint texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width,
			                          face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
			                          face->glyph->bitmap.buffer);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			Character character = {
				texture,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				static_cast<GLuint>(face->glyph->advance.x)
			};

			characters.insert({c, character});
		}

		FT_Done_Face(face);
		FT_Done_FreeType(ft);

		initialized_ = true;

	}

	void FontAtlas::render_text(Shader& s, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
	{
		using namespace glm;
		s.use();
		s.set("textColor", color);

		glActiveTexture(GL_TEXTURE0);
		vao.bind();

		for (auto c : text) {
			Character ch = characters[c];

			GLfloat xpos = x + ch.bearing.x * scale;
			GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;

			GLfloat w = ch.size.x * scale;
			GLfloat h = ch.size.y * scale;

			GLfloat vertices[6][4] = {
				{ xpos, ypos + h, 0.0f, 0.0f },
				{ xpos, ypos, 0.0f, 1.0f },
				{ xpos + w, ypos, 1.0f, 1.0f },

				{ xpos, ypos + h, 0.0f, 0.0f },
				{ xpos + w, ypos, 1.0f, 1.0f },
				{ xpos + w, ypos + h, 1.0f, 0.0f }
			};

			glBindTexture(GL_TEXTURE_2D, ch.tex);
			vbo.bind();
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
			vbo.unbind();

			glDrawArrays(GL_TRIANGLES, 0, 6);
			x += (ch.advance >> 6) * scale;
		}

		vao.unbind();
		glBindTexture(GL_TEXTURE_2D, 0);

	}
}

