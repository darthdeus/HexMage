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

#include <glm/glm.hpp>

using namespace model;
using namespace glm;

namespace game {
	void handlePlayerStep(Sint32 sym, model::GameInstance& game, model::Mob& player) {
		switch (sym) {
		case 'a':
			player.move(game.arena, { -1, 0 });
			break;

		case 'd':
			player.move(game.arena, { 1, 0 });
			break;

		case 'z':
			player.move(game.arena, { 0, -1 });
			break;

		case 'e':
			player.move(game.arena, { 0, 1 });
			break;

		case 'c':
			player.move(game.arena, { 1, -1 });
			break;

		case 'q':
			player.move(game.arena, { -1, 1 });
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
		return arena.hex_near({ view_mouse.x, view_mouse.y });
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

		//st.start();
		//for (int i = 0; i < 100; i++) {
		//	arena.dijkstra({ 1, 1 });
		//}
		//st.print("100x dijkstra");

		Mob& player = info.add_mob(generator::random_mob());
		player.c = { 0, 0 };

		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		gl::Vertex::setup_attributes();

		gl::Batch batch;

		gl::Shader shaderProgram{ "vertex.glsl", "fragment.glsl" };
		shaderProgram.use();

		Coord highlight_hex, mouse_hex;

		std::vector<Coord> highlight_path;
		gl::Camera camera;

		SDL_GL_SetSwapInterval(1);
		SDL_Event windowEvent;

		while (true) {
			while (SDL_PollEvent(&windowEvent)) {
				//ImGui_ImplSdlGL3_ProcessEvent(&windowEvent);

				if (windowEvent.type == SDL_MOUSEMOTION) {
					highlight_hex = hex_at_mouse(camera.projection(), arena, windowEvent.motion.x, windowEvent.motion.y);
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
					auto click_hex = hex_at_mouse(camera.projection(), arena, windowEvent.motion.x, windowEvent.motion.y);

					if (arena(click_hex) == HexType::Empty) {
						arena(click_hex) = HexType::Wall;
					} else {
						arena(click_hex) = HexType::Empty;
					}
					arena.regenerate_geometry();
					arena.dijkstra(player.c);
				}

				if (windowEvent.type == SDL_MOUSEBUTTONDOWN && windowEvent.button.button == SDL_BUTTON_LEFT) {
					auto click_hex = hex_at_mouse(camera.projection(), arena, windowEvent.motion.x, windowEvent.motion.y);

					player.c = click_hex;
					highlight_hex = player.c;
					arena.dijkstra(player.c);
					highlight_path.clear();
				}

				if (windowEvent.type == SDL_QUIT ||
					(windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE))
					return;

				if (windowEvent.type == SDL_KEYDOWN) camera.keydown(windowEvent.key.keysym.sym);
				if (windowEvent.type == SDL_KEYUP) camera.keyup(windowEvent.key.keysym.sym);
				if (windowEvent.type == SDL_MOUSEWHEEL) camera.scroll(windowEvent.wheel.y);
			}

			ImGui_ImplSdlGL3_NewFrame(window);

			camera.update_camera();

			GLint uniTrans = glGetUniformLocation(shaderProgram.program, "trans");
			glUniformMatrix4fv(uniTrans, 1, GL_FALSE, camera.value_ptr());

			glClearColor(0.3f, 0.2f, 0.3f, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			arena.draw_vertices();

			for (auto& mob : info.mobs) {
				auto pos = arena.pos(mob.c);
				paint_at(pos, Arena::radius, color_for_type(HexType::Player));
			}

			auto highlight_pos = arena.pos(highlight_hex);
			paint_at(highlight_pos, Arena::radius, color_for_type(HexType::Player));

			Color highlight_color{ 0.85f, 0.75f, 0.85f };
			auto mouse_pos = arena.pos(mouse_hex);
			paint_at(mouse_pos, Arena::radius, highlight_color);

			for (Coord c : highlight_path) {
				paint_at(arena.pos(c), Arena::radius, highlight_color);
			}

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