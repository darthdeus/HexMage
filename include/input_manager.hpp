#ifndef EVENT_MANAGER_HPP
#define EVENT_MANAGER_HPP

#pragma once
#include "gl_utils.hpp"
#include "model.hpp"

class InputManager
{
	void mousemove(glm::vec2 pos, model::Mob& player);
	void left_click(glm::vec2 pos, model::Mob& player);
	void right_click(glm::vec2 pos, model::Mob& player);

	gl::Camera& camera_;
	model::GameInstance& game_;
	model::Arena& arena_;
	model::PlayerInfo& info_;
	model::TurnManager& turn_manager_;
public:
	SDL_Event event;
	model::Coord highlight_hex;
	model::Coord mouse_hex;
	std::vector<model::Coord> highlight_path;

	InputManager(gl::Camera& camera, model::GameInstance& game, model::Arena& arena, model::PlayerInfo& info, model::TurnManager& turn_manager)
		: camera_(camera),
		  game_(game),
		  arena_(arena),
		  info_(info),
		  turn_manager_(turn_manager) {}

	std::vector<model::Coord> build_highlight_path(const model::Mob& mob);
	bool handle_events();
};

#endif