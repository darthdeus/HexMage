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

#include <glm/glm.hpp>

using namespace model;
using namespace glm;

namespace game
{
	void handlePlayerStep(Sint32 sym, model::GameInstance& game, model::Mob& player) {
		switch (sym) {
			case 'a':
				player.move(game.arena, {-1, 0});
				break;

			case 'd':
				player.move(game.arena, {1, 0});
				break;

			case 'z':
				player.move(game.arena, {0, -1});
				break;

			case 'e':
				player.move(game.arena, {0, 1});
				break;

			case 'c':
				player.move(game.arena, {1, -1});
				break;

			case 'q':
				player.move(game.arena, {-1, 1});
				break;
		}
	}

	void paint_at(Position pos, float radius, Color color) {
		gl::Batch b;
		b.push_hex(pos, color, radius);
		b.draw_arrays();
	}

	Coord hex_at_mouse(const mat4& proj, Arena& arena, int x, int y) {
		auto rel_mouse = mouse2gl(x, y);
		auto view_mouse = inverse(proj) * vec4(rel_mouse.x, rel_mouse.y, 0.0f, 1.0f);
		return arena.hex_near({view_mouse.x, view_mouse.y});
	}

	void game_loop(SDL_Window* window) {
		using namespace model;
		using namespace glm;

		ImGui_ImplSdlGL3_Init(window);

		Stopwatch st;

		GameInstance game(30);
		Arena& arena = game.arena;
		PlayerInfo& info = game.info;

		st.start();
		arena.regenerate_geometry();
		st.print("Arena vertices");

		Mob& player = info.add_mob(generator::random_mob());
		player.c = {0, 0};

		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		gl::Vertex::setup_attributes();

		gl::Shader shader{"vertex.glsl", "fragment.glsl"};
		shader.use();

		gl::Camera camera;

		SDL_GL_SetSwapInterval(1);

		InputManager input_manager;
		while (true) {
			glClearColor(0.3f, 0.2f, 0.3f, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			ImGui_ImplSdlGL3_NewFrame(window);

			bool keep_running = input_manager.handle_events(camera, arena, player);
			if (!keep_running) {
				break;
			}

			camera.update_and_load_camera();
			arena.draw_vertices();

			auto highlight_pos = arena.pos(input_manager.highlight_hex);
			paint_at(highlight_pos, Arena::radius, color_for_type(HexType::Player));

			Color highlight_color{0.85f, 0.75f, 0.85f};
			auto mouse_pos = arena.pos(input_manager.mouse_hex);
			paint_at(mouse_pos, Arena::radius, highlight_color);

			for (Coord c : input_manager.highlight_path) {
				paint_at(arena.pos(c), Arena::radius, highlight_color);
			}

			gl::Batch b;

			for (auto& mob : info.mobs) {
				auto pos = arena.pos(mob.c);
				paint_at(pos, Arena::radius, color_for_type(HexType::Player));

				Healthbar::draw(pos, b, 0.1f);
			}

			b.draw_arrays();

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

			SDL_GL_SwapWindow(window);
		}
	}
}
