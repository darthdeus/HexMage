// Some of the includes include windef.h, which in turn
// defines min/max macros in the global namespace, which will clash
// with the std::min/max functions.
#define NOMINMAX

// Enable math constants
#define _USE_MATH_DEFINES

#include <algorithm>
#include <string>
#include <iostream>
#include <math.h>

#include <glad/glad.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include <game.hpp>


int main(int, char**) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		std::cerr << "Unable to initialize SDL_Init " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	SDL_Window* window = SDL_CreateWindow(
		"HexMage", 300, 300, // TODO - better default screen position
		game::SCREEN_WIDTH, game::SCREEN_HEIGHT, SDL_WINDOW_OPENGL);

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

	game::game_loop(window);

	SDL_GL_DeleteContext(context);
	SDL_Quit();

	return 0;
}

