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
#include <iomanip>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include "stopwatch.hpp"
#include "gl_utils.hpp"
#include <tgaimage.h>
#include "mob.hpp"

const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 768;

using namespace model;
using namespace glm;


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

void paint_at(model::Position pos, float radius, color color) {
	std::vector<float> player_vertices;
	hex_at(player_vertices, pos, radius, color);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * player_vertices.size(), player_vertices.data(), GL_STATIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, player_vertices.size() / 3);

}

Position mouse2gl(Sint32 x, Sint32 y) {
	float rel_x = static_cast<float>(x) / SCREEN_WIDTH;
	float rel_y = static_cast<float>(y) / SCREEN_HEIGHT;

	rel_y = 2 * (1 - rel_y) - 1;
	rel_x = 2 * rel_x - 1;

	return{ rel_x, rel_y };
}

void game_loop(SDL_Window* window) {
	// TODO - proc tohle nefunguje?
	// glEnable(GL_POLYGON_SMOOTH | GL_MULTISAMPLE);
	using namespace model;
	using namespace glm;

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	ShaderProgram program{ "vertex.glsl", "fragment.glsl" };
	std::cerr << glGetError() << std::endl;


	GameInstance game(30);
	Arena& arena = game.arena;
	PlayerInfo& info = game.info;

	stopwatch ss;

	ss.start();
	arena.regenerate_geometry();
	std::cout << "Arena vertices in: " << ss.ms() << "ms" << std::endl;

	ss.start();
	for (int i = 0; i < 100; i++) {
		arena.dijkstra({ 1, 1 });
	}
	std::cout << "100x dijkstra done in " << ss.ms() << "ms" << std::endl;

	Mob& player = game.info.add_mob(generator::random_mob());

	stopwatch st;
	stopwatch st_frame;

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	program.setupAttributes();

	Coord highlight_hex;
	std::vector<Coord> highlight_path;

	Position translate{ 0, 0 };
	Position rel_mouse{ 0,0 };

	Position current_scroll{ 0, 0 };

	Coord selected_hex;
	float zoom_level = 0.7;

	mat4 zoom{ 1 };
	mat4 mov{ 1 };
	mat4 projection_mat{ 1 };

	SDL_Event windowEvent;
	while (true) {
		st_frame.start();
		st.start();

		const float scroll_zone = 0.9f;
		const float scroll_offset = 0.05f;

		while (SDL_PollEvent(&windowEvent)) {
			if (windowEvent.type == SDL_MOUSEMOTION) {
				rel_mouse = mouse2gl(windowEvent.motion.x, windowEvent.motion.y);
				auto view_mouse = inverse(projection_mat) * vec4(rel_mouse.x, rel_mouse.y, 0.0f, 1.0f);
				highlight_hex = arena.hex_near({ view_mouse.x, view_mouse.y });

				highlight_path.clear();

				while (highlight_hex != player.c) {
					highlight_path.push_back(highlight_hex);
					highlight_hex = arena.paths(highlight_hex).source;
				}
			}

			if (windowEvent.type == SDL_MOUSEBUTTONDOWN && windowEvent.button.button == SDL_BUTTON_RIGHT) {
				auto rel_pos = mouse2gl(windowEvent.motion.x, windowEvent.motion.y);
				auto view_mouse = inverse(projection_mat) * vec4(rel_pos.x, rel_pos.y, 0.0f, 1.0f);
				auto click_hex = arena.hex_near({ view_mouse.x, view_mouse.y });

				if (arena(click_hex) == HexType::Empty) {
					arena(click_hex) = HexType::Wall;
				}
				else {
					arena(click_hex) = HexType::Empty;
				}
				arena.regenerate_geometry();
				arena.dijkstra(player.c);
			}

			if (windowEvent.type == SDL_MOUSEBUTTONDOWN && windowEvent.button.button == SDL_BUTTON_LEFT) {
				auto rel_pos = mouse2gl(windowEvent.motion.x, windowEvent.motion.y);
				auto view_mouse = inverse(projection_mat) * vec4(rel_pos.x, rel_pos.y, 0.0f, 1.0f);
				auto click_hex = arena.hex_near({ view_mouse.x, view_mouse.y });

				player.c = click_hex;
				highlight_hex = player.c;
				arena.dijkstra(player.c);
				highlight_path.clear();
			}

			if (windowEvent.type == SDL_QUIT ||
				(windowEvent.type == SDL_KEYUP &&
					windowEvent.key.keysym.sym == SDLK_ESCAPE))
				return;

			if (windowEvent.type == SDL_KEYDOWN) {
				switch (windowEvent.key.keysym.sym) {
				case 'w':
					current_scroll.y = -scroll_offset;
					break;
				case 's':
					current_scroll.y = scroll_offset;
					break;
				case 'a':
					current_scroll.x = scroll_offset;
					break;
				case 'd':
					current_scroll.x = -scroll_offset;
					break;
				}
			}

			if (windowEvent.type == SDL_KEYUP) {
				switch (windowEvent.key.keysym.sym) {
				case 'w':
				case 's':
					current_scroll.y = 0;
					break;

				case 'a':
				case 'd':
					current_scroll.x = 0;
					break;
				}
			}

			if (windowEvent.type == SDL_MOUSEWHEEL) {
				zoom_level += 0.07f * windowEvent.wheel.y;
			}
		}

		translate += current_scroll;

		mov = glm::translate(mat4(1.0f), vec3(static_cast<vec2>(translate), 0));
		zoom = glm::scale(mat4(1.0f), vec3(zoom_level));

		projection_mat = zoom * mov;

		GLint uniTrans = glGetUniformLocation(program.shaderProgram, "trans");
		glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(projection_mat));

		glClearColor(0.3f, 0.2f, 0.3f, 1);
		glClear(GL_COLOR_BUFFER_BIT);


		arena.draw_vertices();

		for (auto& mob : info.mobs) {
			auto pos = arena.pos(mob.c);
			paint_at(pos, Arena::radius, color_for_type(HexType::Player));
		}

		auto highlight_pos = arena.pos(highlight_hex);
		paint_at(highlight_pos, Arena::radius, color_for_type(HexType::Player));

		for (Coord c : highlight_path) {
			paint_at(arena.pos(c), Arena::radius, { 0.85f, 0.75f, 0.85f });
		}

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

