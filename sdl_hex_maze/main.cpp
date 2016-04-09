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
#include "hex_grid.hpp"
#include <tgaimage.h>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;


void handlePlayerStep(Sint32 sym, HexGrid& grid) {
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
}

void paint_at(float x, float y, float radius, color color) {
	std::vector<float> player_vertices;
	hex_at(player_vertices, x, y, radius, color_for_type(HexType::Player));
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * player_vertices.size(), player_vertices.data(), GL_STATIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, player_vertices.size() / 3);

}

Coord mouse2gl(Sint32 x, Sint32 y) {
	float rel_x = static_cast<float>(x) / SCREEN_WIDTH;
	float rel_y = static_cast<float>(y) / SCREEN_HEIGHT;

	rel_y = 2 * (1 - rel_y) - 1;
	rel_x = 2 * rel_x - 1;

	return{ rel_x, rel_y };
}

void game_loop(SDL_Window* window) {
	// TODO - proc tohle nefunguje?
	// glEnable(GL_POLYGON_SMOOTH | GL_MULTISAMPLE);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	ShaderProgram program{"vertex.glsl", "fragment.glsl"};
	std::cerr << glGetError() << std::endl;

	float radius = 0.1f;
	
	HexGrid grid{30};

	grid.move_player(0, 0);
	grid(0, 1) = HexType::Wall;
	grid(0, 2) = HexType::Wall;

	grid.prebuild_VBO(radius);

	stopwatch st;
	stopwatch st_frame;

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	program.setupAttributes();

	std::pair<int, int> highlight_hex;
	
	SDL_Event windowEvent;
	while (true) {
		st_frame.start();
		st.start();

		while (SDL_PollEvent(&windowEvent)) {
			if (windowEvent.type == SDL_MOUSEMOTION) {
				Coord c = mouse2gl(windowEvent.button.x, windowEvent.button.y);
				
				highlight_hex = grid.highlight_near(c);
			}

			if (windowEvent.type == SDL_QUIT ||
				(windowEvent.type == SDL_KEYUP &&
					windowEvent.key.keysym.sym == SDLK_ESCAPE))
				return;

			if (windowEvent.type == SDL_KEYDOWN) {
				handlePlayerStep(windowEvent.key.keysym.sym, grid);
			}
		}

		glClearColor(0.3f, 0.2f, 0.3f, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * grid.vertices.size(), grid.vertices.data(), GL_STATIC_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, grid.vertices.size() / 3);

		auto player_pos = grid.pos(grid.player_col, grid.player_row);
		paint_at(player_pos.first, player_pos.second, radius, color_for_type(HexType::Player));
		
		auto highlight_pos = grid.pos(highlight_hex.first, highlight_hex.second);
		paint_at(highlight_pos.first, highlight_pos.second, radius, color_for_type(HexType::Wall));

		SDL_GL_SwapWindow(window);

		std::cout << "Frame: " << st_frame.ms() << "ms" << std::endl;
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
	SDL_Quit();

	return 0;
}

