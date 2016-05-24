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

#include <lodepng.h>
#include <format.h>
#include <glm/glm.hpp>

#include <stopwatch.hpp>
#include <gl_utils.hpp>
// #include <model.hpp>
// #include <simulation.hpp>

#include <config.hpp>
#include <input_manager.hpp>
#include <game.hpp>

// TODO - put these under a subdirectory for more readable includes
#include <sim.hpp>
#include <generator.hpp>
#include <map_geometry.hpp>

using namespace sim;
using namespace glm;

namespace game
{
	void draw_imgui();

  glm::vec2 mouse2gl(int x, int y) {
		float rel_x = static_cast<float>(x) / SCREEN_WIDTH;
		float rel_y = static_cast<float>(y) / SCREEN_HEIGHT;

		rel_y = 2 * (1 - rel_y) - 1;
		rel_x = 2 * rel_x - 1;

		return{ rel_x, rel_y };
	}


  sim::Coord hex_at_mouse(const mat4& proj, MapGeometry& geom, int x, int y)
	{
		auto rel_mouse = mouse2gl(x, y);
		auto view_mouse = inverse(proj) * vec4(rel_mouse.x, rel_mouse.y, 0.0f, 1.0f);
		return geom.hex_near({view_mouse.x, view_mouse.y});
	}

	static void draw_abilities(Game& game)
	{
    auto&& turn_manager = game.turn_manager();

		if (!turn_manager.is_turn_done()) {
			auto&& mob = turn_manager.current_mob();

			// auto target = game.info.can_attack(player, input_manager.mouse_hex);

			ImGui::Begin("Current player");

			ImGui::Text("HP: %d/%d\nAP: %d/%d", mob.hp, mob.max_hp, mob.ap, mob.max_ap);

			for (auto&& ability : game.usable_abilities(mob)) {
				std::string usable = "";
				// if (target) {
				// 	if (player->can_use_ability_at(*target, game.info, game.arena, ability)) {
				// 		usable = "* ";
				// 	}
				// }

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

      // TODO - add this back
			// if (target) {
			// 	auto tm = target->mob;
			// 	ImGui::Text("HP: %d/%d\nAP: %d/%d", tm.hp, tm.max_hp, tm.ap, tm.max_ap);
			// }

			ImGui::End();
		}
	}

	void game_loop(SDL_Window* window)
	{
		using namespace sim;
		using namespace glm;

		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		ImGui_ImplSdlGL3_Init(window);

    sim::Game game(20);

    // TODO - User vs AI player?
		auto t1 = game.add_team();
		auto t2 = game.add_team();

		for (int i = 0; i < 10; i++) {
			auto t = i < 5 ? t1 : t2;
			game.add_mob(gen::random_mob(t, game.size()));
		}

    auto&& turn_manager = game.turn_manager();
    turn_manager.start_next_turn();

    assert(!turn_manager.is_turn_done());

    auto&& mob = turn_manager.current_mob();

    auto&& pathfinder = game.pathfinder();
    pathfinder.pathfind_from(mob.c, game.map(), game.mob_manager());

    MapGeometry geom(game);
    geom.regenerate_geometry(mob.ap);

		gl::Camera camera;

		gl::Texture2D t;
		t.image_format = GL_RGBA;
		t.internal_format = GL_RGBA;
		t.load_png("res/chicken.png");

		float WIDTH = (float)SCREEN_WIDTH;
		float HEIGHT = (float)SCREEN_HEIGHT;

		gl::SpriteRenderer sprites;

		gl::FontRenderer fonts;
		fonts.set_projection(glm::ortho(0.f, WIDTH, 0.0f, HEIGHT));

		auto projection = ortho(0.f, WIDTH, HEIGHT, 0.0f);

		InputManager input_manager(camera, game, geom);

		while (true) {
			glClearColor(0.3f, 0.2f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			ImGui_ImplSdlGL3_NewFrame(window);

			bool keep_running = input_manager.handle_events();
			if (!keep_running) {
				break;
			}
			camera.update_camera();

			fonts.render_text("HexMage", 10, 37, 42);
			fonts.render_text("HexMage", 10, 20, 22);
			fonts.render_text("HexMage", 10, 10, 12);

			sprites.set_projection(camera.projection() * projection);
			sprites.draw_sprite(t, {0, 0}, {32, 32});
			sprites.draw_sprite(t, {32, 32}, {64, 64});

			geom.set_projection(camera.projection());
			geom.draw_vertices();

			auto highlight_pos = geom.pos(input_manager.highlight_hex);
			geom.paint_hex(highlight_pos, MapGeometry::radius, color_for_type(sim::HexType::Player));

      glm::vec4 highlight_color{0.75f, 0.15f, 0.35f, 0.9f};
			auto mouse_pos = geom.pos(input_manager.mouse_hex);
			geom.paint_hex(mouse_pos, MapGeometry::radius, highlight_color);

			// TODO - only show highlight_path if there's a player controlled team
			//for (Coord c : input_manager.highlight_path) {
			//	geom.paint_hex(geom.pos(c), geom::radius, highlight_color);
			//}

			for (auto&& mob : game.mob_manager().mobs()) {
				geom.paint_mob(mob);
			}

			draw_abilities(game);

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
    ImGui::Text("TODO - profiling is temporarily disabled");
		// if (ImGui::Button("Dummy profile")) {
		// 	simulation::dummy_profiling();
		// }
    //
		// if (simulation::profiling_results.size() > 0) {
		// 	for (auto& res : simulation::profiling_results) {
		// 		ImGui::Text(res.c_str());
		// 	}
		// }

		ImGui::End();
		ImGui::Render();
	}
}
