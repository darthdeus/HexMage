// Some of the includes include windef.h, which in turn
// defines min/max macros in the global namespace, which will clash
// with the std::min/max functions.
#define NOMINMAX

#include <algorithm>
#include <string>
#include <cmath>
#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <random>
#include <unordered_map>
#include <math.h>
#include <glm/glm.hpp>

#include <glad/glad.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include "gl_utils.hpp"
#include <tgaimage.h>
#include "mob.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

float rad_for_hex(int i) {
	float angle_deg = static_cast<float>(60 * i + 30);
	return M_PI / 180 * angle_deg;
}

void push_vertex(std::vector<float>& vbo, float x, float y, color c) {
	vbo.push_back(x);
	vbo.push_back(y);
	push_color(vbo, c.r, c.g, c.b, c.a);
}

void hex_at(std::vector<float>& vertices, model::Position pos, float r, color c) {
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

class mat
{
	std::size_t mat_dim_;
	matrix<model::HexType> data_;
	matrix<std::pair<float, float>> positions_;

public:
	int m;
	int player_row = 0, player_col = 0;
	bool player_set = false;

	mat(int m)
		: mat_dim_(m),
		  data_(mat_dim_, mat_dim_),
		  positions_(mat_dim_, mat_dim_),
		  m(m) {
		// TODO - ugly
		std::fill(positions_.vs.begin(), positions_.vs.end(),
		          std::make_pair<float, float>(INFINITY, INFINITY));
	}

	mat(const mat&) = delete;

	model::HexType& operator()(int col, int row) {
		return data_(col, row);
	}

	std::pair<float, float>& pos(int col, int row) {
		return positions_(col, row);
	}

	bool move_player(int col, int row) {
		if (fmax(std::abs(row), std::abs(col)) >= m || fmin(row, col) < 0)
			return false;
		if ((*this)(col, row) == model::HexType::Wall)
			return false;

		//if (player_set) {
		//	(*this)(player_col, player_row) = HexType::Empty;
		//}

		player_row = row;
		player_col = col;

		//(*this)(col, row) = HexType::Player;

		player_set = true;
		return true;
	}

	bool step_player(int dcol, int drow) {
		if (player_set) {
			return move_player(player_col + dcol, player_row + drow);
		}
		return false;
	}

	std::pair<int, int> highlight_near(float rel_x, float rel_y) {
		int closest_x = 2;
		int closest_y = 2;
		float min = INFINITY;

		for (size_t i = 0; i < positions_.m; i++) {
			for (size_t j = 0; j < positions_.n; j++) {
				//if ((*this)(i, j) == HexType::Player)
				//	(*this)(i, j) = HexType::Empty;

				auto pos = positions_(i, j);
				float d1 = pos.first - rel_x;
				float d2 = pos.second - rel_y;

				float distance = d1 * d1 + d2 * d2;
				if (distance < min) {
					closest_x = static_cast<int>(j);
					closest_y = static_cast<int>(i);
					min = distance;
				}
			}
		}

		return {closest_y, closest_x};
		//(*this)(closest_y, closest_x) = HexType::Player;
	}
};

void handlePlayerStep(Sint32 sym, model::GameInstance& game, model::Mob& player) {
	switch (sym) {
		case 'a':
			player.move(game.arena, { -1, 0 });
			break;

		case 'd':
			player.move(game.arena, { 1, 0 });
			break;

		case 'z':
			player.move(game.arena, { 0, -1 });
			break;

		case 'e':
			player.move(game.arena, { 0, 1 });
			break;

		case 'c':
			player.move(game.arena, { 1, -1 });
			break;

		case 'q':
			player.move(game.arena, { -1, 1 });
			break;
	}
}

color color_for_type(model::HexType type) {
	using namespace model;
	switch (type) {
		case HexType::Empty:
			return {0.4f, 0.2f, 0.4f};
		case HexType::Wall:
			return {0.1f, 0.03f, 0.1f};
		case HexType::Player:
			return {0.7f, 0.4f, 0.7f};
		default:
			throw "invalid hex type";
	}
}

void paint_at(model::Position pos, float radius, color color) {
	std::vector<float> player_vertices;
	hex_at(player_vertices, pos, radius, color_for_type(model::HexType::Player));
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * player_vertices.size(), player_vertices.data(), GL_STATIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, player_vertices.size() / 3);

}

void game_loop(SDL_Window* window) {
	// TODO - proc tohle nefunguje?
	// glEnable(GL_POLYGON_SMOOTH | GL_MULTISAMPLE);
	using namespace model;

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	ShaderProgram program{"vertex.glsl", "fragment.glsl"};
	std::cerr << glGetError() << std::endl;

	GameInstance game(30);
	Arena& arena = game.arena;
	PlayerInfo& info = game.info;

	Mob& player = game.info.add_mob(generator::random_mob());	

	float start_x = -0.5f;
	float start_y = -0.5f;

	float radius = 0.1f;
	float width = cos(30 * M_PI / 180) * radius * 2;
	float height_offset = radius + sin(30 * M_PI / 180) * radius;

	std::vector<float> vertices;
	vertices.reserve(1000);

	for (int row = 0; row < game.size; ++row) {
		for (int col = 0; col < game.size; ++col) {
			float draw_x = start_x;
			float draw_y = start_y;

			// axial q-change
			draw_x += col * width;
			// axial r-change
			draw_x += row * (width / 2);
			draw_y += row * height_offset;

			arena.pos({ col, row }) = { draw_x, draw_y };

			color c = color_for_type(arena({ col, row }));

			auto pos = arena.pos({ col, row });
			hex_at(vertices, pos, radius, c);
			c = c.mut(0.004f);
		}
	}

	stopwatch st;
	stopwatch st_frame;

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	program.setupAttributes();

	Coord highlight_hex;

	SDL_Event windowEvent;
	while (true) {
		st_frame.start();
		st.start();


		while (SDL_PollEvent(&windowEvent)) {
			if (windowEvent.type == SDL_MOUSEMOTION) {
				float rel_x = static_cast<float>(windowEvent.motion.x) / SCREEN_WIDTH;
				float rel_y = static_cast<float>(windowEvent.motion.y) / SCREEN_HEIGHT;

				rel_y = 2 * (1 - rel_y) - 1;
				rel_x = 2 * rel_x - 1;

				highlight_hex = arena.highlight_near({ rel_x, rel_y });
			}

			if (windowEvent.type == SDL_QUIT ||
				(windowEvent.type == SDL_KEYUP &&
					windowEvent.key.keysym.sym == SDLK_ESCAPE))
				return;

			if (windowEvent.type == SDL_KEYDOWN) {
				handlePlayerStep(windowEvent.key.keysym.sym, game, player);
			}
		}

		glClearColor(0.3f, 0.2f, 0.3f, 1);
		glClear(GL_COLOR_BUFFER_BIT);


		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);

		auto player_pos = arena.pos(player.c);
		paint_at(player_pos, radius, color_for_type(HexType::Player));

		auto highlight_pos = arena.pos(highlight_hex);
		paint_at(highlight_pos, radius, color_for_type(HexType::Wall));

		SDL_GL_SwapWindow(window);

		// std::cout << "Frame: " << st_frame.ms() << "ms" << std::endl;
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

