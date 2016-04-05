#include <algorithm>
#include <string>
#include <cmath>
#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <random>
#include <unordered_map>

#include <glad/glad.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include "gl_utils.hpp"
#include <tgaimage.h>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

enum class HexType
{
	Empty = 0,
	Player,
	Wall
};


struct hash_int_triple {
  std::size_t operator()(const std::tuple<int, int, int>& tup) const {
    using namespace std;

    return hash<int>()(get<0>(tup)) ^ (hash<int>()(get<1>(tup)) << 1) ^
           (hash<int>()(get<2>(tup)) << 1);
  }
};

struct hash_int_pair {
  std::size_t operator()(const std::pair<int, int>& tup) const {
    using namespace std;

    return hash<int>()(tup.first) ^ (hash<int>()(tup.second) << 1);
  }
};

class mat
{
	std::unordered_map<std::tuple<int, int, int>, HexType, hash_int_triple> data;

public:
	int m;
	int player_x, player_y, player_z;
	bool player_set = false;

	mat(int m) : m(m) {}

	mat(const mat&) = delete;

	HexType& operator()(int x, int y, int z) {
		return data[std::make_tuple(x, y, z)];
	}

	bool move_player(int x, int y, int z) {
		if (max(x, max(y, z)) >= m)
			return false;
		if ((*this)(x, y, z) == HexType::Wall)
			return false;

		if (player_set) {
			(*this)(player_x, player_y, player_z) = HexType::Empty;
		}

		player_x = x;
		player_y = y;
		player_z = z;

		(*this)(x, y, z) = HexType::Player;

		player_set = true;
		return true;
	}

	bool step_player(int dx, int dy, int dz) {
		if (player_set) {
			return move_player(player_x + dx, player_y + dy, player_z + dz);
		}
		return false;
	}
};

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

#define RZ(x, m) (clamp((x)+rnd(m) - (m) / 2))

struct seed
{
	float x, y, r, g, b, a;

	seed(float x, float y, float r, float g, float b, float a)
		: x{x}, y{y}, r{r}, g{g}, b{b}, a{a} {}

	void mutate(bool pos) {
		float m = 0.05f / 60.0f;

		// std::cout << rnd(0.3f) << "\t" << x << "\t";
		float r = rnd(m) - m / 2;
		if (pos) {
			x = RZ(x, m * 10);
			y = RZ(y, m * 10);
		} else {
			x = RZ(x, m);
			y = RZ(y, m);
		}
		r = RZ(r, m);
		g = RZ(g, m);
		b = RZ(b, m);
		a = RZ(a, m);
		// std::cout << "\t" << r << "\t" << x << std::endl;
		// y = clamp(y + rnd(m) - m / 2);
	}
};

void triangle_at(ShaderProgram& program, seed& s) {
	VBO vbo{program};

	float off = 0.07f;
	color c{s.r, s.g, s.b, s.a};
	vbo.push_vertex(s.x, s.y + off, c);
	vbo.push_vertex(s.x + off, s.y - off, c);
	vbo.push_vertex(s.x - off, s.y - off, c);

	vbo.draw(GL_TRIANGLES);
}

float rad_for_hex(int i) {
	float angle_deg = 60 * i + 30;
	return M_PI / 180 * angle_deg;
}

void hex_at(ShaderProgram& program, float x, float y, float r, color c) {
	VBO vbo{program};

	vbo.push_vertex(x, y, c);

	int rot = 0; // 1;
	for (int i = rot; i < 7 + rot; i++) {
		float ri = rad_for_hex(i);
		c = c.mut(0.03f);
		vbo.push_vertex(x + r * cos(ri), y + r * sin(ri), c);
	}

	vbo.draw(GL_TRIANGLE_FAN);
}

seed s(0.2f, 0.2f, 0.3f, 0.4f, 0.2f, 0.5f);

void updateGeometry(ShaderProgram& program) {
	// hex_at(0, 0, 10, { 1, 1, 1 });

	for (int i = 0; i < 1; i++) {
		triangle_at(program, s);
		s.mutate(true);
	}
}

void game_loop(SDL_Window* window) {
	std::vector<float> vertices;

	// TODO - proc tohle nefunguje?
	// glEnable(GL_POLYGON_SMOOTH | GL_MULTISAMPLE);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	ShaderProgram program{"vertex.glsl", "fragment.glsl"};
	std::cerr << glGetError() << std::endl;

	mat grid{5};

	for (int i = 1; i < 5; i++) {
		grid(1, i, -1 - i) = HexType::Wall;
	}

	grid(-1, 1, 0) = HexType::Wall;
	grid(0, -1, 1) = HexType::Wall;
	grid(-1, 0, 1) = HexType::Wall;

	grid.move_player(0, 0, 0);
	grid(1, 0, 0) = HexType::Wall;

	TGAImage img;
	int width, height;

	if (!img.read_tga_file("sample.tga")) {
		std::cerr << "Nepovedlo se" << std::endl;
	}

	width = img.get_width() * 2;
	height = img.get_height();
	unsigned char* image = img.buffer();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	SDL_Event windowEvent;
	while (true) {
		if (SDL_PollEvent(&windowEvent)) {
			if (windowEvent.type == SDL_MOUSEMOTION) {
				std::cout << windowEvent.motion.x << " " << windowEvent.motion.y << std::endl;
			}

			if (windowEvent.type == SDL_QUIT ||
				(windowEvent.type == SDL_KEYUP &&
					windowEvent.key.keysym.sym == SDLK_ESCAPE))
				break;

			if (windowEvent.type == SDL_KEYDOWN) {
				switch (windowEvent.key.keysym.sym) {
					case 'a':
						grid.step_player(-1, 1, 0);
						break;

					case 'd':
						grid.step_player(1, -1, 0);
						break;

					case 'z':
						grid.step_player(0, 1, -1);
						break;

					case 'e':
						grid.step_player(0, -1, 1);
						break;

					case 'c':
						grid.step_player(1, 0, -1);
						break;

					case 'q':
						grid.step_player(-1, 0, 1);
						break;
				}
			}
		}

		glClearColor(0.3f, 0.2f, 0.3f, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		color c_wall = {0.1f, 0.03f, 0.1f};
		color c_empty = {0.4f, 0.2f, 0.4f};
		color c_player = {0.7f, 0.4f, 0.7f};

		// float start_x = -0.8;
		// float start_y = 0.8;

		float start_x = 0;
		float start_y = 0;

		float r = 0.1f;
		float height = 2 * r;
		float width = cos(30 * M_PI / 180) * r * 2;
		float height_offset = r + sin(30 * M_PI / 180) * r;

		std::unordered_map<std::pair<int, int>, std::tuple<int, int, int>, hash_int_pair> taken;

		// for (int x = -0; x < 1; x++) {
		for (int x = -grid.m; x < grid.m; x++) {
			// for (int y = -0; y < 1; y++) {
			for (int y = -grid.m; y < grid.m; y++) {
				for (int z = -grid.m; z < grid.m; z++) {
					if (x + y + z != 0)
						continue;

					int q = x;
					int rr = z;

					float draw_x = start_x;
					float draw_y = start_y;

					// axial q-change
					draw_x += q * width;
					// axial r-change
					draw_x += rr * (width / 2);
					draw_y += rr * height_offset;

					int dkey1 = (int)round(draw_x * 10000);
					int dkey2 = (int)round(draw_y * 10000);

					auto key = std::make_pair(dkey1, dkey2);
					if (taken.count(key) > 0) {
						auto by = taken[key];
						std::cout << "ALREADY TAKEN " << dkey1 << " " << dkey2 << "\t" << x
								<< " " << y << " " << z << "\t" << std::get<0>(by) << " "
								<< std::get<1>(by) << " " << std::get<2>(by) << std::endl;
						throw "he";
					} else {
						taken[key] = std::make_tuple(x, y, z);
					}

					color c;

					switch (grid(x, y, z)) {
						case HexType::Empty:
							c = c_empty;
							break;
						case HexType::Wall:
							c = c_wall;
							break;
						case HexType::Player:
							c = c_player;
							break;
					}

					hex_at(program, draw_x, draw_y, r, c);
					c = c.mut(0.004f);
				}
			}
		}
		// std::cout << std::endl << std::endl;

		SDL_GL_SwapWindow(window);
	}
}

int main(int argc, char** argv) {
	// TODO - error handling
	SDL_Init(SDL_INIT_VIDEO);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	// TODO - error handling
	SDL_Window* window = SDL_CreateWindow(
		"OpenGL", 300, 300, // TODO - better default screen position
		SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);

	// TODO - error handling
	SDL_GLContext context = SDL_GL_CreateContext(window);

	gladLoadGLLoader(SDL_GL_GetProcAddress);

	game_loop(window);

	SDL_GL_DeleteContext(context);
	// SDL_Delay(1000);
	SDL_Quit();

	return 0;
}

