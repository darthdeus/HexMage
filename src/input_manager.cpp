#include <glad/glad.h>
#include <SDL/SDL.h>
#include <input_manager.hpp>
#include <imgui_impl_sdl.h>
#include <game.hpp>

using namespace model;
using namespace glm;

void InputManager::mousemove(glm::vec2 pos, Mob& player)
{
	highlight_hex = game::hex_at_mouse(camera_.projection(), arena_, event.motion.x, event.motion.y);
	mouse_hex = highlight_hex;

	highlight_path = build_highlight_path(player);
}

void InputManager::left_click(glm::vec2 pos, Mob& current_mob)
{
	auto click_hex = game::hex_at_mouse(camera_.projection(), arena_, event.motion.x, event.motion.y);
	
	auto&& player = current_mob.team->player();

	if (player.is_ai()) {
		fmt::print("Forcing AI to take a turn\n");
		player.any_action(game_, current_mob);
	} else {
		player.action_to(click_hex, game_, current_mob);

		highlight_hex = current_mob.c;
		highlight_path.clear();

	}

	arena_.dijkstra(current_mob.c, info_);
	arena_.regenerate_geometry(current_mob.ap);
}

void InputManager::right_click(glm::vec2 pos, Mob& player)
{
	auto click_hex = game::hex_at_mouse(camera_.projection(), arena_, event.motion.x, event.motion.y);

	if (arena_(click_hex) == HexType::Empty) {
		arena_(click_hex) = HexType::Wall;
	} else {
		arena_(click_hex) = HexType::Empty;
	}
	arena_.dijkstra(player.c, info_);
	arena_.regenerate_geometry();
}

std::vector<model::Coord>
InputManager::build_highlight_path(const model::Mob& player)
{
	using namespace model;
	std::vector<model::Coord> path;

	if (arena_(highlight_hex) != HexType::Wall) {
		while (highlight_hex != player.c) {
			if (arena_(highlight_hex) == HexType::Wall) {
				path.clear();
				break;
			}

			if (auto source = arena_.paths(highlight_hex).source) {
				highlight_hex = *source;
				if (arena_(highlight_hex) == HexType::Empty) {
					path.push_back(highlight_hex);
				}
			} else {
				path.clear();
				break;
			}
		}
	}

	return path;
}

bool InputManager::handle_events()
{
	using namespace model;

	while (SDL_PollEvent(&event)) {
		ImGui_ImplSdlGL3_ProcessEvent(&event);

		// TODO - rewrite this
		auto& turn = turn_manager_.current_turn;
		if (!turn.is_done()) {
			auto& player = **turn_manager_.current_turn.current_;

			switch (event.type) {
				case SDL_MOUSEMOTION:
					mousemove({event.motion.x, event.motion.y}, player);
					break;

				case SDL_MOUSEBUTTONDOWN:
					glm::vec2 pos{event.motion.x, event.motion.y};
					switch (event.button.button) {
						case SDL_BUTTON_LEFT:
							left_click(pos, player);
							break;
						case SDL_BUTTON_RIGHT:
							right_click(pos, player);
							break;
					}
			}

			if (event.type == SDL_QUIT ||
				(event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE))
				return false;

		}

		if (event.type == SDL_KEYDOWN)
			camera_.keydown(event.key.keysym.sym);
		if (event.type == SDL_KEYUP)
			if (event.key.keysym.sym == SDLK_SPACE) {
				// TODO - dijkstra for current player
				auto* next_player = turn_manager_.current_turn.next();
				if (next_player) {
					arena_.dijkstra(next_player->c, info_);
					arena_.regenerate_geometry(next_player->ap);
				}

				if (turn_manager_.current_turn.is_done()) {
					// TODO - use proper logging
					fmt::print("DEBUG - starting new turn\n");
					turn_manager_.current_turn = game_.start_turn();
				}
			} else {
				camera_.keyup(event.key.keysym.sym);
			}
		if (event.type == SDL_MOUSEWHEEL)
			camera_.scroll(event.wheel.y);
	}

	return true;
}
