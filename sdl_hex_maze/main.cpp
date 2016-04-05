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
	matrix(unsigned m, unsigned n) : m(m), n(n), vs(m * n) {}

	T& operator ()(unsigned i, unsigned j) {
		return vs[n * i + j];
	}

private:
	unsigned m;
	unsigned n;
	std::vector<T> vs;
}; /* column-major/opengl: vs[i + m * j], row-major/c++: vs[n * i + j] */

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


float rad_for_hex(int i) {
	float angle_deg = 60 * i + 30;
	return M_PI / 180 * angle_deg;
}

void hex_at(ShaderProgram& program, float x, float y, float r, color c) {
	VBO vbo{ program };

	vbo.push_vertex(x, y, c);

	int rot = 0; // 1;
	for (int i = rot; i < 7 + rot; i++) {
		float ri = rad_for_hex(i);
		c = c.mut(0.03f);
		vbo.push_vertex(x + r * cos(ri), y + r * sin(ri), c);
	}

	vbo.draw(GL_TRIANGLE_FAN);
}
class mat
{
	matrix<HexType> data_;

public:
	int m;
	int player_row = 0, player_col = 0;
	bool player_set = false;

	mat(int m) : data_(2 * m + 1, 2 * m + 1), m(m) {}

	mat(const mat&) = delete;

	HexType& operator()(int col, int row) {
		// TODO - nema to byt obracene?
		return data_(col + m, row + m);
	}

	HexType& operator()(int x, int y, int z) {
		// TODO - check this
		return data_(x + m, z + m);
	}

	bool move_player(int col, int row) {
		if (max(std::abs(row), std::abs(col)) >= m)
			return false;
		if ((*this)(col, row) == HexType::Wall)
			return false;

		if (player_set) {
			(*this)(player_col, player_row) = HexType::Empty;
		}

		player_row = row;
		player_col = col;

		(*this)(col, row) = HexType::Player;

		player_set = true;
		return true;
	}

	bool step_player(int dcol, int drow) {
		if (player_set) {
			return move_player(player_col + dcol, player_row + drow);
		}
		return false;
	}
};


void handlePlayerStep(Sint32 sym, mat& grid) {
	switch (sym) {
	case 'a':
		grid.step_player(-1, 0);
		break;

	case 'd':
		grid.step_player(1, 0);
		break;

	case 'z':
		grid.step_player(0, -1);
		break;

	case 'e':
		grid.step_player(0, 1);
		break;

	case 'c':
		grid.step_player(1, -1);
		break;

	case 'q':
		grid.step_player(-1, 1);
		break;
	}
	std::cout << "col " << grid.player_col << "\trow " << grid.player_row << std::endl;

}

color color_for_type(HexType type) {
	color c_wall = { 0.1f, 0.03f, 0.1f };
	color c_empty = { 0.4f, 0.2f, 0.4f };
	color c_player = { 0.7f, 0.4f, 0.7f };

	switch (type) {
	case HexType::Empty:
		return c_empty;
	case HexType::Wall:
		return c_wall;
	case HexType::Player:
		return c_player;
	default:
		throw "invalid hex type";
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

	grid.move_player(0, 0);
	grid(0, 1) = HexType::Wall;
	grid(0, 2) = HexType::Wall;

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
				handlePlayerStep(windowEvent.key.keysym.sym, grid);
				
			}
		}

		glClearColor(0.3f, 0.2f, 0.3f, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		float start_x = 0;
		float start_y = 0;

		float radius = 0.1f;
		float width = cos(30 * M_PI / 180) * radius * 2;
		float height_offset = radius + sin(30 * M_PI / 180) * radius;

		for (int row = -grid.m; row < grid.m; ++row) {
			for (int col = -grid.m; col < grid.m; ++col) {

				float draw_x = start_x;
				float draw_y = start_y;

				// axial q-change
				draw_x += col * width;
				// axial r-change
				draw_x += row * (width / 2);
				draw_y += row * height_offset;

				color c = color_for_type(grid(col, row));

				hex_at(program, draw_x, draw_y, radius, c);
				c = c.mut(0.004f);
			}
		}

		//// for (int x = -0; x < 1; x++) {
		//for (int x = -grid.m; x < grid.m; x++) {
		//	// for (int y = -0; y < 1; y++) {
		//	for (int y = -grid.m; y < grid.m; y++) {
		//		for (int z = -grid.m; z < grid.m; z++) {
		//			if (x + y + z != 0)
		//				continue;

		//			int q = x;
		//			int rr = z;

		//			float draw_x = start_x;
		//			float draw_y = start_y;

		//			// axial q-change
		//			draw_x += q * width;
		//			// axial r-change
		//			draw_x += rr * (width / 2);
		//			draw_y += rr * height_offset;

		//			color c = color_for_type(grid(x, y, z));

		//			hex_at(program, draw_x, draw_y, r, c);
		//			c = c.mut(0.004f);
		//		}
		//	}
		//}

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

