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

#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"

unsigned char ttf_buffer[1 << 20];
unsigned char temp_bitmap[512 * 512];

stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
GLuint ftex;
void my_stbtt_initfont(void)
{
	fread(ttf_buffer, 1, 1 << 20, fopen("c:/windows/fonts/times.ttf", "rb"));
	//fread(ttf_buffer, 1, 1 << 20, fopen("res/ProggyClean.ttf", "rb"));
	stbtt_BakeFontBitmap(ttf_buffer, 0, 32.0, temp_bitmap, 512, 512, 32, 96, cdata); // no guarantee this fits!

	lodepng::encode("foo.png", temp_bitmap, 512, 512, LCT_GREY);
	// can free ttf_buffer at this point
	glGenTextures(1, &ftex);
	glBindTexture(GL_TEXTURE_2D, ftex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA, 512, 512, 0, GL_BGR, GL_UNSIGNED_BYTE, temp_bitmap);
	// can free temp_bitmap at this point
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void my_stbtt_print(float x, float y, char* text, glm::mat4 proj)
{
	gl::Batch b;
	// assume orthographic projection with units = screen pixels, origin at top left
	//glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ftex);

	while (*text++) {
		if (*text >= 32 && *text < 128) {
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(cdata, 512, 512, *text - 32, &x, &y, &q, 1);

			//fmt::printf("%f,%f\t%f,%f\n", q.x0, q.y0, q.s0, q.t1);
			float z = -0.5f;

			//fmt::printf("%f %f %f %f\t%f %f %f %f\n", q.x0, q.y0, q.x1, q.y1, q.s0, q.t0, q.s1, q.t1);
			auto v = proj * glm::vec4(q.x0, q.y0, 0, 1);
			auto v2 = proj * glm::vec4(q.x1, q.y1, 0, 1);
			//fmt::printf("\t%f %f %f %f\n", v.x, v.y, v2.x, v2.y);

			b.push_back({{q.x0, -q.y0, z}, {0, 1} });
			b.push_back({{q.x1, -q.y0, z}, {1, 1,}});
			b.push_back({{q.x1, q.y1, z},  {1, 0}});

			b.push_back({{q.x0, -q.y0, z}, {0, 1}});
			b.push_back({{q.x1, q.y1, z},  {1, 0}});
			b.push_back({{q.x0, q.y1, z},  {0, 0}});

			//b.push_back({{q.x0, -q.y0, z}, {q.s0, q.t1}});
			//b.push_back({{q.x1, -q.y0, z}, {q.s1, q.t1}});
			//b.push_back({{q.x1, q.y1, z},  {q.s1, q.t0}});

			//b.push_back({{q.x0, -q.y0, z}, {q.s0, q.t1}});
			//b.push_back({{q.x1, q.y1, z},  {q.s1, q.t0}});
			//b.push_back({{q.x0, q.y1, z},  {q.s0, q.t0}});
		}
	}

	b.draw_arrays();
	//glBegin(GL_QUADS);
	//while (*text) {
	//	if (*text >= 32 && *text < 128) {
	//		stbtt_aligned_quad q;
	//		stbtt_GetBakedQuad(cdata, 512, 512, *text - 32, &x, &y, &q, 1);//1=opengl & d3d10+,0=d3d9
	//		glTexCoord2f(q.s0, q.t1); glVertex2f(q.x0, q.y0);
	//		glTexCoord2f(q.s1, q.t1); glVertex2f(q.x1, q.y0);
	//		glTexCoord2f(q.s1, q.t0); glVertex2f(q.x1, q.y1);
	//		glTexCoord2f(q.s0, q.t0); glVertex2f(q.x0, q.y1);
	//	}
	//	++text;
	//}
	//glEnd();
}

using namespace model;
using namespace glm;

namespace game
{
	void paint_at(Position pos, float radius, Color color)
	{
		gl::Batch b;
		b.push_hex(pos, color, radius);
		b.draw_arrays();
	}

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
		glEnable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

		gl::Camera camera;

		SDL_GL_SetSwapInterval(1);
		my_stbtt_initfont();

		gl::Texture2D t;
		t.image_format = GL_RGBA;
		t.internal_format = GL_RGBA;
		t.load_png("res/chicken.png");

		gl::Shader spriteShader("res/sprite.vs.glsl", "res/sprite.fs.glsl");
		auto projection = glm::ortho(0.f, (float)game::SCREEN_WIDTH, (float)game::SCREEN_HEIGHT, 0.f, -1.f, 1.f);
		gl::SpriteRenderer sprites{ spriteShader };

		InputManager input_manager;
		while (true) {
			glClearColor(0.3f, 0.2f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			ImGui_ImplSdlGL3_NewFrame(window);

			bool keep_running = input_manager.handle_events(camera, arena, player);
			if (!keep_running) {
				break;
			}
			camera.update_camera();

			spriteShader.set("projection", camera.projection() * projection);
			t.bind();
			sprites.draw_sprite(t, {0, 0}, {32, 32});
			sprites.draw_sprite(t, {32, 32}, {32, 32});

			arena.shader.use();
			camera.update_and_load_camera();
			arena.draw_vertices();

			//gl::Batch b1;
			//b1.push_back(gl::Vertex({ 0, 0, 0 }, { 1,1,1,1 }, { 0, 0 }, 1));
			//b1.push_back(gl::Vertex({ 1, 0, 0 }, { 1,1,1,1 }, { 1, 0 }, 1));
			//b1.push_back(gl::Vertex({ 0, 1, 0 }, { 1,1,1,1 }, { 0, 1 }, 1));

			//b1.draw_arrays();

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

				Healthbar::draw(pos, b, 0.7f, 0.5f);
			}

			b.draw_arrays();

			arena.vao.bind();
			arena.vbo.bind();
			arena.shader.use();
			arena.shader.set("projection", glm::translate(glm::scale(projection, glm::vec3(10, 10, 1)), glm::vec3(20, 10, 0)));
			my_stbtt_print(0, 0, "hell ole", projection);
			arena.shader.set("projection", glm::mat4(1.0f));

			//ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiSetCond_FirstUseEver);
			//ImGui::Begin("Framerate");
			//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			//ImGui::End();

			//ImGui::Begin("Profiling");
			//if (ImGui::Button("Dummy profile")) {
			//	simulation::dummy_profiling();
			//}

			//if (simulation::profiling_results.size() > 0) {
			//	for (auto& res : simulation::profiling_results) {
			//		ImGui::Text(res.c_str());
			//	}
			//}

			//ImGui::End();
			//ImGui::Render();

			SDL_GL_SwapWindow(window);
		}
	}
}
