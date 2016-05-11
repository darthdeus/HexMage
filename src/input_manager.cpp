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

void InputManager::left_click(glm::vec2 pos, Mob& player)
{
	auto click_hex = game::hex_at_mouse(camera_.projection(), arena_, event.motion.x, event.motion.y);

	auto path = arena_.paths(click_hex);

	if (auto target = info_.can_attack(player, click_hex)) {
		if (player.ap >= 5) {
			target->mob.hp -= 5;
			player.ap -= 5;
		}
	} else {
		if (path.distance <= player.ap) {
			player.ap -= path.distance;
			player.c = click_hex;

			highlight_hex = player.c;
			highlight_path.clear();
		}
	}

	arena_.dijkstra(player.c);
	arena_.regenerate_geometry(player.ap);
}

void InputManager::right_click(glm::vec2 pos, Mob& player)
{
	auto click_hex = game::hex_at_mouse(camera_.projection(), arena_, event.motion.x, event.motion.y);

	if (arena_(click_hex) == HexType::Empty) {
		arena_(click_hex) = HexType::Wall;
	} else {
		arena_(click_hex) = HexType::Empty;
	}
	arena_.dijkstra(player.c);
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
				if (highlight_hex != player.c) {
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

			if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT) { }

			if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) { }

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
					arena_.dijkstra(next_player->c);
					arena_.regenerate_geometry(next_player->ap);
				}
			} else {
				camera_.keyup(event.key.keysym.sym);
			}
		if (event.type == SDL_MOUSEWHEEL)
			camera_.scroll(event.wheel.y);
	}

	return true;
}
