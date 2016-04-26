// Some of the includes include windef.h, which in turn
// defines min/max macros in the global namespace, which will clash
// with the std::min/max functions.
#define NOMINMAX

#include <algorithm>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
		std::vector<float> player_vertices;
		hex_at(player_vertices, pos, radius, color);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * player_vertices.size(), player_vertices.data(), GL_STATIC_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(player_vertices.size()) / 3);
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

		st.start();
		for (int i = 0; i < 100; i++) {
			arena.dijkstra({ 1, 1 });
		}
		st.print("100x dijkstra");

		Mob& player = game.info.add_mob(generator::random_mob());

		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		
		GLsizei stride = 6 * sizeof(GLfloat);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<GLvoid*>(0));
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<GLvoid*>(2 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

		//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<GLvoid*>(6 * sizeof(GLfloat)));
		//glEnableVertexAttribArray(2);

		gl::Shader shaderProgram{ "vertex.glsl", "fragment.glsl" };
		shaderProgram.use();

		Coord highlight_hex;
		std::vector<Coord> highlight_path;

		Position translate{ 0, 0 };

		Position current_scroll{ 0, 0 };

		float zoom_level = 0.7f;

		mat4 zoom{ 1 };
		mat4 mov{ 1 };
		mat4 projection_mat{ 1 };

		SDL_Event windowEvent;
		while (true) {
			const float scroll_offset = 0.05f;

			while (SDL_PollEvent(&windowEvent)) {
				ImGui_ImplSdlGL3_ProcessEvent(&windowEvent);

				if (windowEvent.type == SDL_MOUSEMOTION) {
					highlight_hex = hex_at_mouse(projection_mat, arena, windowEvent.motion.x, windowEvent.motion.y);
					highlight_path.clear();

					while (highlight_hex != player.c) {
						highlight_path.push_back(highlight_hex);
						highlight_hex = arena.paths(highlight_hex).source;
					}
				}

				if (windowEvent.type == SDL_MOUSEBUTTONDOWN && windowEvent.button.button == SDL_BUTTON_RIGHT) {
					auto click_hex = hex_at_mouse(projection_mat, arena, windowEvent.motion.x, windowEvent.motion.y);

					if (arena(click_hex) == HexType::Empty) {
						arena(click_hex) = HexType::Wall;
					}
					else {
						arena(click_hex) = HexType::Empty;
					}
					arena.regenerate_geometry();
					arena.dijkstra(player.c);
				}

				if (windowEvent.type == SDL_MOUSEBUTTONDOWN && windowEvent.button.button == SDL_BUTTON_LEFT) {
					auto click_hex = hex_at_mouse(projection_mat, arena, windowEvent.motion.x, windowEvent.motion.y);

					player.c = click_hex;
					highlight_hex = player.c;
					arena.dijkstra(player.c);
					highlight_path.clear();
				}

				if (windowEvent.type == SDL_QUIT ||
					(windowEvent.type == SDL_KEYUP &&
						windowEvent.key.keysym.sym == SDLK_ESCAPE))
					return;

				if (windowEvent.type == SDL_KEYDOWN) {
					switch (windowEvent.key.keysym.sym) {
					case 'w':
						current_scroll.y = -scroll_offset;
						break;
					case 's':
						current_scroll.y = scroll_offset;
						break;
					case 'a':
						current_scroll.x = scroll_offset;
						break;
					case 'd':
						current_scroll.x = -scroll_offset;
						break;
					}
				}

				if (windowEvent.type == SDL_KEYUP) {
					switch (windowEvent.key.keysym.sym) {
					case 'w':
					case 's':
						current_scroll.y = 0;
						break;

					case 'a':
					case 'd':
						current_scroll.x = 0;
						break;
					}
				}

				if (windowEvent.type == SDL_MOUSEWHEEL) {
					zoom_level += 0.07f * windowEvent.wheel.y;
				}
			}

			ImGui_ImplSdlGL3_NewFrame(window);

			translate += current_scroll;

			mov = glm::translate(mat4(1.0f), vec3(static_cast<vec2>(translate), 0));
			zoom = glm::scale(mat4(1.0f), vec3(zoom_level));

			projection_mat = zoom * mov;

			GLint uniTrans = glGetUniformLocation(shaderProgram.program, "trans");
			glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(projection_mat));

			glClearColor(0.3f, 0.2f, 0.3f, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			arena.draw_vertices();

			for (auto& mob : info.mobs) {
				auto pos = arena.pos(mob.c);
				paint_at(pos, Arena::radius, color_for_type(HexType::Player));
			}

			auto highlight_pos = arena.pos(highlight_hex);
			paint_at(highlight_pos, Arena::radius, color_for_type(HexType::Player));

			for (Coord c : highlight_path) {
				paint_at(arena.pos(c), Arena::radius, { 0.85f, 0.75f, 0.85f });
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