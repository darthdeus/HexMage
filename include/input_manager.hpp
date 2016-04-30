#ifndef EVENT_MANAGER_HPP
#define EVENT_MANAGER_HPP

#pragma once
#include "gl_utils.hpp"
#include "model.hpp"

class InputManager
{
public:
	SDL_Event windowEvent;
	Coord highlight_hex;
	Coord mouse_hex;
	std::vector<Coord> highlight_path;

	bool handle_events(::gl::Camera& camera, model::Arena& arena, model::Mob& player);
};

#endif