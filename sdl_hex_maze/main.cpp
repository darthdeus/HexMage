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

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

class ShaderSource
{
	std::string filename_;
	std::string contents_;
	const char* c_str_;
	const GLchar** source_;

public:
	ShaderSource(std::string filename): filename_(filename)
	{
		std::ifstream is{ filename };

		contents_ = {
			std::istreambuf_iterator<char>(is),
			std::istreambuf_iterator<char>() };

		c_str_ = contents_.c_str();
		source_ = &c_str_;
	}


	const GLchar** source()
	{
		return source_;
	}

	GLuint compile(GLenum type)
	{
		GLuint shaderID = glCreateShader(type);
		glShaderSource(shaderID, 1, source(), nullptr);
		glCompileShader(shaderID);

		GLint status;
		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
		if (status != GL_TRUE)
			std::cerr << "Shader from " << filename_ << " failed to compile" << std::endl;

		return shaderID;
	}
};

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
	printf("Vendor:   %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version:  %s\n", glGetString(GL_VERSION));

	float vertices[] = {
		-0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // Top-left
		0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // Top-right
		0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // Bottom-right
		-0.5f, -0.5f, 1.0f, 1.0f, 1.0f  // Bottom-left
	};

	GLuint elements[] = {
		0,1,2,
		2,3,0
	};

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLuint ebo;
	glGenBuffers(1, &ebo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	ShaderSource vertexShaderSource{ "vertex.glsl" };
	ShaderSource fragmentShaderSource{ "fragment.glsl" };

	GLuint vertexShader = vertexShaderSource.compile(GL_VERTEX_SHADER);
	GLuint fragmentShader = fragmentShaderSource.compile(GL_FRAGMENT_SHADER);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");

	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);

	GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
	glEnableVertexAttribArray(colAttrib);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

	GLint uniColor = glGetUniformLocation(shaderProgram, "triangleColor");
	glUniform3f(uniColor, 1.0f, 0.0f, 0.0f);

	std::cerr << glGetError() << std::endl;

	auto t_start = std::chrono::high_resolution_clock::now();

	SDL_Event windowEvent;
	while (true)
	{
		if (SDL_PollEvent(&windowEvent))
		{
			if (windowEvent.type == SDL_QUIT || (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)) break;
		}

		auto t_now = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

		glUniform3f(uniColor, (sin(time * 4.0f) + 1.0f) / 2.0f, 0.0f, 0.0f);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		//glDrawArrays(GL_TRIANGLES, 0, 3);

		SDL_GL_SwapWindow(window);
	}

	SDL_GL_DeleteContext(context);
	//SDL_Delay(1000);
	SDL_Quit();


	return 0;
}

