#ifndef GAME_HPP
#define GAME_HPP

#pragma once

// Some of the includes include windef.h, which in turn
// defines min/max macros in the global namespace, which will clash
// with the std::min/max functions.
#define NOMINMAX

#include <SDL/SDL.h>

#include <map_geometry.hpp>
#include <gl_utils.hpp>

namespace game {
sim::Coord hex_at_mouse(const glm::mat4 &proj, MapGeometry &game,
                        glm::ivec2 pos);
void game_loop(SDL_Window *window);
}

#endif
