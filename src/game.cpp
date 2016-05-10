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

	void game_loop(SDL_Window* window)
	{
		using namespace model;
		using namespace glm;

		glViewport(0, 0, game::SCREEN_WIDTH, game::SCREEN_HEIGHT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		ImGui_ImplSdlGL3_Init(window);

		Stopwatch st;

		GameInstance game(30);
		Arena& arena = game.arena;
		PlayerInfo& info = game.info;

		UserPlayer user_player;
		Team t1(1, user_player);

		AIPlayer ai_player;
		Team t2(2, ai_player);

		arena.regenerate_geometry();

		Mob& player = info.add_mob(generator::random_mob());
		player.c = {0, 0};

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

		InputManager input_manager;
		while (true) {
			glClearColor(0.3f, 0.2f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			ImGui_ImplSdlGL3_NewFrame(window);

			bool keep_running = input_manager.handle_events(camera, arena, player, info);
			if (!keep_running) {
				break;
			}
			camera.update_camera();

			// jenom pro demo
			fonts.render_text("HexMage", 10, 10, 12);
			fonts.render_text("HexMage", 10, 40, 52);

			sprites.set_projection(camera.projection() * projection);
			sprites.draw_sprite(t, {0, 0}, {32, 32});
			sprites.draw_sprite(t, {32, 32}, {32, 32});

			arena.set_projection(camera.projection());
			arena.draw_vertices();

			auto highlight_pos = arena.pos(input_manager.highlight_hex);
			arena.paint_hex(highlight_pos, Arena::radius, color_for_type(HexType::Player));

			Color highlight_color{0.85f, 0.75f, 0.85f};
			auto mouse_pos = arena.pos(input_manager.mouse_hex);
			arena.paint_hex(mouse_pos, Arena::radius, highlight_color);

			for (Coord c : input_manager.highlight_path) {
				arena.paint_hex(arena.pos(c), Arena::radius, highlight_color);
			}

			for (auto& mob : info.mobs) {
				auto pos = arena.pos(mob.c);
				arena.paint_hex(pos, Arena::radius, color_for_type(HexType::Player));
				arena.paint_healthbar(pos, 0.7f, 0.5f);
			}

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
