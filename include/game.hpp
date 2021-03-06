#ifndef GAME_HPP
#define GAME_HPP

#pragma once

// Some of the includes include windef.h, which in turn
// defines min/max macros in the global namespace, which will clash
// with the std::min/max functions.
#define NOMINMAX

#include <model.hpp>
#include <gl_utils.hpp>
#include <SDL/SDL.h>

namespace game
{
	model::Coord hex_at_mouse(const glm::mat4& proj, model::Arena& arena, int x, int y);
	void game_loop(SDL_Window* window);

}

#endif