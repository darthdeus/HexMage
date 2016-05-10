#include <glad/glad.h>
#include <SDL/SDL.h>
#include <input_manager.hpp>
#include <imgui_impl_sdl.h>
#include <game.hpp>

bool InputManager::handle_events(gl::Camera& camera, model::Arena& arena, model::Mob& player, model::PlayerInfo& info) {
	using namespace model;

	while (SDL_PollEvent(&windowEvent)) {
		ImGui_ImplSdlGL3_ProcessEvent(&windowEvent);

		if (windowEvent.type == SDL_MOUSEMOTION) {
			highlight_hex = game::hex_at_mouse(camera.projection(), arena, windowEvent.motion.x, windowEvent.motion.y);
			mouse_hex = highlight_hex;
			highlight_path.clear();

			if (arena(highlight_hex) != HexType::Wall) {
				while (highlight_hex != player.c) {
					if (arena(highlight_hex) == HexType::Wall) {
						highlight_path.clear();
						break;
					}

					if (auto source = arena.paths(highlight_hex).source) {
						highlight_hex = *source;
						if (highlight_hex != player.c) {
							highlight_path.push_back(highlight_hex);
						}
					} else {
						highlight_path.clear();
						break;
					}
				}
			}
		}

		if (windowEvent.type == SDL_MOUSEBUTTONDOWN && windowEvent.button.button == SDL_BUTTON_RIGHT) {
			auto click_hex = game::hex_at_mouse(camera.projection(), arena, windowEvent.motion.x, windowEvent.motion.y);

			if (arena(click_hex) == HexType::Empty) {
				arena(click_hex) = HexType::Wall;
			} else {
				arena(click_hex) = HexType::Empty;
			}
			arena.regenerate_geometry();
			arena.dijkstra(player.c);
		}

		if (windowEvent.type == SDL_MOUSEBUTTONDOWN && windowEvent.button.button == SDL_BUTTON_LEFT) {
			auto click_hex = game::hex_at_mouse(camera.projection(), arena, windowEvent.motion.x, windowEvent.motion.y);

			player.c = click_hex;
			highlight_hex = player.c;
			arena.dijkstra(player.c);
			highlight_path.clear();
		}

		if (windowEvent.type == SDL_QUIT ||
			(windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE))
			return false;

		if (windowEvent.type == SDL_KEYDOWN)
			camera.keydown(windowEvent.key.keysym.sym);
		if (windowEvent.type == SDL_KEYUP)
			camera.keyup(windowEvent.key.keysym.sym);
		if (windowEvent.type == SDL_MOUSEWHEEL)
			camera.scroll(windowEvent.wheel.y);
	}

	return true;
}
