#include <iostream>
#include <glad/glad.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <GL/gl.h>
#include <string>
#include <stdio.h>
#include <fstream>
#include <chrono>
#include <vector>
#include <random>
#include <stdlib.h>

#include "gl_utils.hpp"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;


float rnd(float max)
{
	return (static_cast<float>(rand()) / RAND_MAX) * max;
}

float rnd() { return rnd(1.0f); }

float clamp(float x)
{
	if (x < 0) return 0;
	if (x > 1) return 1;
	return x;
}

#define RZ(x, m) (clamp((x) + rnd(m) - (m) / 2))

struct color
{
	float r, g, b, a;
	color(float r, float g, float b) : r(r), g(g), b(r), a(1) {}
	color(float r, float g, float b, float a): r(r), g(g), b(r), a(a) {}
};

struct seed
{
	float x, y, r, g, b, a;
	seed(float x, float y, float r, float g, float b, float a) : x{ x }, y{ y }, r{ r }, g{ g }, b{ b }, a{ a } {}

	void mutate(bool pos)
	{
		float m = 0.05f / 60.0f;		

		//std::cout << rnd(0.3f) << "\t" << x << "\t";
		float r = rnd(m) - m / 2;
		if (pos) {
			x = RZ(x, m * 10);
			y = RZ(y, m * 10);
		} else {
			x = RZ(x, m);
			y = RZ(y, m);
		}
		r = RZ(r, m);
		g = RZ(g, m);
		b = RZ(b, m);
		a = RZ(a, m);
		//std::cout << "\t" << r << "\t" << x << std::endl;
		//y = clamp(y + rnd(m) - m / 2);
	}
};

void push_color(std::vector<float>& v, float r, float g, float b, float a)
{
	v.push_back(r);
	v.push_back(g);
	v.push_back(b);
	v.push_back(a);
}

void triangle_at(std::vector<float>& v, seed& s)
{
	float off = 0.07;
	v.push_back(s.x);
	v.push_back(s.y + off);
	push_color(v, s.r, s.g, s.b, s.a);

	v.push_back(s.x + off);
	v.push_back(s.y - off);
	push_color(v, s.r, s.g, s.b, s.a);

	v.push_back(s.x - off);
	v.push_back(s.y - off);
	push_color(v, s.r, s.g, s.b, s.a);
}

seed s(0.2, 0.2, 0.3, 0.4, 0.2, 0.5);

void updateGeometry(std::vector<float>& v)
{
	v.clear();

	for (int i = 0; i < 200; i++)
	{
		triangle_at(v, s);
		s.mutate(true);
	}
}

void game_loop(SDL_Window* window)
{
	//float vertices[] = {
	//	-0.5f, 0.5f, 1.0f, 0.0f, 0.0f, // Top-left
	//	0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // Top-right
	//	0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // Bottom-right

	//	-0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // Top-left
	//	0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // Bottom-right
	//	-0.5f, -0.5f, 1.0f, 1.0f, 1.0f  // Bottom-left
	//};	

	std::vector<float> vertices;

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	ShaderProgram program{ "vertex.glsl", "fragment.glsl" };
	GLuint shaderProgram = program.shaderProgram;

	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);

	GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
	glEnableVertexAttribArray(colAttrib);
	glVertexAttribPointer(colAttrib, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));

	GLint uniColor = glGetUniformLocation(shaderProgram, "triangleColor");
	glUniform3f(uniColor, 1.0f, 0.0f, 0.0f);
	
	glBindFragDataLocation(shaderProgram, 0, "outColor");


	auto t_start = std::chrono::high_resolution_clock::now();

	SDL_Event windowEvent;
	while (true)
	{
		//std::cerr << glGetError() << std::endl;

		if (SDL_PollEvent(&windowEvent))
		{
			if (windowEvent.type == SDL_QUIT || (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)) break;
		}

		updateGeometry(vertices);

		glBufferData(GL_ARRAY_BUFFER,
			sizeof(float) * vertices.size(),
			vertices.data(), GL_STATIC_DRAW);

		glClearColor(0.3f, 0.2f, 0.3f, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		auto t_now = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

		//glUniform3f(uniColor, (sin(time * 4.0f) + 1.0f) / 2.0f, 0.0f, 0.0f);

		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		SDL_GL_SwapWindow(window);
	}
}

int main(int argc, char** argv)
{
	// TODO - error handling
	SDL_Init(SDL_INIT_VIDEO);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	// TODO - error handling
	SDL_Window* window = SDL_CreateWindow("OpenGL",
		300, 300, // TODO - better default screen position
		800, 600,
		SDL_WINDOW_OPENGL);

	// TODO - error handling
	SDL_GLContext context = SDL_GL_CreateContext(window);

	gladLoadGLLoader(SDL_GL_GetProcAddress);
	
	game_loop(window);
	
	SDL_GL_DeleteContext(context);
	//SDL_Delay(1000);
	SDL_Quit();


	return 0;
}
