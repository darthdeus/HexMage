#ifndef EVENT_MANAGER_HPP
#define EVENT_MANAGER_HPP

#pragma once
#include <SDL/SDL.h>

#include "map_geometry.hpp"
#include "gl_utils.hpp"
#include "sim.hpp"

class InputManager
{
	void mousemove(glm::vec2 pos, sim::Mob& player);
	void left_click(glm::vec2 pos, sim::Mob& player);
	void right_click(glm::vec2 pos, sim::Mob& player);

	gl::Camera& camera_;
	sim::Game& game_;
  MapGeometry& geom_;
public:
	SDL_Event event;
	sim::Coord highlight_hex;
	sim::Coord mouse_hex;
	std::vector<sim::Coord> highlight_path;

	InputManager(gl::Camera& camera, sim::Game& game, MapGeometry& geom)
		: camera_(camera),
		  game_(game),
      geom_(geom) {}


	std::vector<sim::Coord> build_highlight_path(const sim::Mob& mob);
	bool handle_events();
};

#endif
