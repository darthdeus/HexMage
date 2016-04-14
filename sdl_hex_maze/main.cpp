// Some of the includes include windef.h, which in turn
// defines min/max macros in the global namespace, which will clash
// with the std::min/max functions.
#define NOMINMAX

// Enable math constants
#define _USE_MATH_DEFINES

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

#include "stopwatch.hpp"
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

void handlePlayerStep(Sint32 sym, model::GameInstance& game, model::Mob& player) {
	switch (sym) {
		case 'a':
			player.move(game.arena, {-1, 0});
			break;

		case 'd':
			player.move(game.arena, {1, 0});
			break;

		case 'z':
			player.move(game.arena, {0, -1});
			break;

		case 'e':
			player.move(game.arena, {0, 1});
			break;

		case 'c':
			player.move(game.arena, {1, -1});
			break;

		case 'q':
			player.move(game.arena, {-1, 1});
			break;
	}
}

color color_for_type(model::HexType type) {
	switch (type) {
		case model::HexType::Empty:
			return {0.4f, 0.2f, 0.4f};
		case model::HexType::Wall:
			return {0.1f, 0.03f, 0.1f};
		case model::HexType::Player:
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

Coord mouse2gl(Sint32 x, Sint32 y) {
	float rel_x = static_cast<float>(x) / SCREEN_WIDTH;
	float rel_y = static_cast<float>(y) / SCREEN_HEIGHT;

	rel_y = 2 * (1 - rel_y) - 1;
	rel_x = 2 * rel_x - 1;

	return {rel_x, rel_y};
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

			arena.pos({col, row}) = {draw_x, draw_y};

			color c = color_for_type(arena({col, row}));

			auto pos = arena.pos({col, row});
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

	model::Coord highlight_hex;

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

				highlight_hex = arena.highlight_near({rel_x, rel_y});
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

		for (auto& mob : info.mobs) {
			auto pos = arena.pos(mob.c);
			paint_at(pos, radius, color_for_type(HexType::Player));
		}

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

	SDL_Window* window = SDL_CreateWindow(
		"OpenGL", 300, 300, // TODO - better default screen position
		SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);

	if (window == nullptr) {
		std::cerr << "Unable to initialize SDL_Window, exiting." << std::endl;
		return 1;
	}

	SDL_GLContext context = SDL_GL_CreateContext(window);
	if (context == nullptr) {
		std::cerr << "Unable to initialize OpenGL context, exiting." << std::endl;
		return 1;
	}

	gladLoadGLLoader(SDL_GL_GetProcAddress);

	game_loop(window);

	SDL_GL_DeleteContext(context);
	SDL_Quit();

	return 0;
}

