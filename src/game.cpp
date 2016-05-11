// Some of the includes include windef.h, which in turn
// defines min/max macros in the global namespace, which will clash
// with the std::min/max functions.
#define NOMINMAX

#include <algorithm>
#include <string>
#include <vector>

#include <glad/glad.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include <imgui.h>
#include <imgui_impl_sdl.h>

#include <format.h>

#include <stopwatch.hpp>
#include <gl_utils.hpp>
#include <model.hpp>
#include <simulation.hpp>
#include <input_manager.hpp>
#include <lodepng.h>

#include <game.hpp>
#include <glm/glm.hpp>

using namespace model;
using namespace glm;

namespace game
{
	void draw_imgui();

	Coord hex_at_mouse(const mat4& proj, Arena& arena, int x, int y)
	{
		auto rel_mouse = mouse2gl(x, y);
		auto view_mouse = inverse(proj) * vec4(rel_mouse.x, rel_mouse.y, 0.0f, 1.0f);
		return arena.hex_near({view_mouse.x, view_mouse.y});
	}

	void draw_abilities(const TurnManager& turn_manager, GameInstance& game, InputManager& input_manager)
	{
		if (!turn_manager.current_turn.is_done()) {
			auto* player = *turn_manager.current_turn.current_;
			auto target = game.info.can_attack(*player, input_manager.mouse_hex);

			ImGui::Begin("Current player");

			ImGui::Text("HP: %d/%d\nAP: %d/%d", player->hp, player->max_hp, player->ap, player->max_ap);

			for (auto&& ability : player->abilities) {
				std::string usable = "";
				if (target) {
					if (player->can_use_ability_at(*target, game.info, game.arena, ability)) {
						usable = "* ";
					}
				}

				auto str = fmt::sprintf(
					"%scost: %d, dmg: %d/%d, range: %d",
					usable,
					ability.cost,
					ability.d_hp,
					ability.d_ap,
					ability.range);

				ImGui::Text(str.c_str());
			}

			ImGui::End();

			ImGui::Begin("Enemy");

			if (target) {
				auto tm = target->mob;
				ImGui::Text("HP: %d/%d\nAP: %d/%d", tm.hp, tm.max_hp, tm.ap, tm.max_ap);
			}

			ImGui::End();
		}
	}

	void game_loop(SDL_Window* window)
	{
		using namespace model;
		using namespace glm;

		glViewport(0, 0, game::SCREEN_WIDTH, game::SCREEN_HEIGHT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		ImGui_ImplSdlGL3_Init(window);

		GameInstance game(20);
		Arena& arena = game.arena;
		PlayerInfo& info = game.info;

		UserPlayer user_player;
		AIPlayer ai_player;

		int t1 = info.register_team(user_player);
		int t2 = info.register_team(ai_player);

		for (int i = 0; i < 10; i++) {
			int t = i < 5 ? t1 : t2;
			Mob& mob = info.add_mob(generator::random_mob(t, arena.size));
		}

		TurnManager turn_manager(info);
		turn_manager.current_turn = game.start_turn();
		turn_manager.update_arena(arena);

		Mob* current_player = *turn_manager.current_turn.current_;
		arena.dijkstra(current_player->c);
		arena.regenerate_geometry(current_player->ap);

		gl::Camera camera;

		gl::Texture2D t;
		t.image_format = GL_RGBA;
		t.internal_format = GL_RGBA;
		t.load_png("res/chicken.png");

		float WIDTH = (float)game::SCREEN_WIDTH;
		float HEIGHT = (float)game::SCREEN_HEIGHT;

		gl::SpriteRenderer sprites;

		gl::FontRenderer fonts;
		fonts.set_projection(ortho(0.f, WIDTH, 0.0f, HEIGHT));

		auto projection = ortho(0.f, WIDTH, HEIGHT, 0.0f);

		InputManager input_manager(camera, arena, info, turn_manager);

		while (true) {
			glClearColor(0.3f, 0.2f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			ImGui_ImplSdlGL3_NewFrame(window);

			bool keep_running = input_manager.handle_events();
			if (!keep_running) {
				break;
			}
			camera.update_camera();

			fonts.render_text("HexMage", 10, 10, 12);

			sprites.set_projection(camera.projection() * projection);
			sprites.draw_sprite(t, {0, 0}, {32, 32});
			sprites.draw_sprite(t, {32, 32}, {32, 32});

			arena.set_projection(camera.projection());
			arena.draw_vertices();

			auto highlight_pos = arena.pos(input_manager.highlight_hex);
			arena.paint_hex(highlight_pos, Arena::radius, color_for_type(HexType::Player));

			Color highlight_color{0.75f, 0.15f, 0.35f, 0.9f};
			auto mouse_pos = arena.pos(input_manager.mouse_hex);
			arena.paint_hex(mouse_pos, Arena::radius, highlight_color);

			for (Coord c : input_manager.highlight_path) {
				arena.paint_hex(arena.pos(c), Arena::radius, highlight_color);
			}

			for (auto& mob : info.mobs) {
				arena.paint_mob(info, mob);
			}

			draw_abilities(turn_manager, game, input_manager);

			draw_imgui();

			SDL_GL_SwapWindow(window);
		}
	}

	void draw_imgui()
	{
		ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Framerate");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();

		ImGui::Begin("Profiling");
		if (ImGui::Button("Dummy profile")) {
			simulation::dummy_profiling();
		}

		if (simulation::profiling_results.size() > 0) {
			for (auto& res : simulation::profiling_results) {
				ImGui::Text(res.c_str());
			}
		}

		ImGui::End();
		ImGui::Render();


	}
}
