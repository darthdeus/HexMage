#include <iostream>
#include <glad/glad.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
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

enum class HexType { Empty = 0, Player, Wall };

class mat {
  std::vector<HexType> data;

 public:
  int m, n;
  int player_i, player_j;
  bool player_set = false;

  mat(int m, int n) : m(m), n(n), data(m * n) {}
  mat(const mat&) = delete;

  HexType& operator()(int i, int j) { return data[i * n + j]; }

  // TODO - fuj, michame tu i/j a x/y
  bool move_player(int i, int j) {
    if (i < 0 || j < 0 || i >= m || j >= n) return false;
    if ((*this)(i, j) == HexType::Wall) return false;

    if (player_set) {
      (*this)(player_i, player_j) = HexType::Empty;
    }
    player_i = i;
    player_j = j;
    (*this)(i, j) = HexType::Player;

    player_set = true;
    return true;
  }

  bool step_player(int dx, int dy) {
    if (player_set) {
      return move_player(player_i + dy, player_j + dx);
    }
    return false;
  }
};

float rnd() { return rnd(1.0f); }

float clamp(float x) {
  if (x < 0) return 0;
  if (x > 1) return 1;
  return x;
}

#define RZ(x, m) (clamp((x)+rnd(m) - (m) / 2))

struct seed {
  float x, y, r, g, b, a;
  seed(float x, float y, float r, float g, float b, float a)
      : x{x}, y{y}, r{r}, g{g}, b{b}, a{a} {}

  void mutate(bool pos) {
    float m = 0.05f / 60.0f;

    // std::cout << rnd(0.3f) << "\t" << x << "\t";
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
    // std::cout << "\t" << r << "\t" << x << std::endl;
    // y = clamp(y + rnd(m) - m / 2);
  }
};

void triangle_at(ShaderProgram& program, seed& s) {
  VBO vbo{program};

  float off = 0.07;
  color c{s.r, s.g, s.b, s.a};
  vbo.push_vertex(s.x, s.y + off, c);
  vbo.push_vertex(s.x + off, s.y - off, c);
  vbo.push_vertex(s.x - off, s.y - off, c);

  vbo.draw(GL_TRIANGLES);
}

float rad_for_hex(int i) {
  float angle_deg = 60 * i + 30;
  return M_PI / 180 * angle_deg;
}

void hex_at(ShaderProgram& program, float x, float y, float r, color c) {
  VBO vbo{program};

  vbo.push_vertex(x, y, c);

  int rot = 0;  // 1;
  for (int i = rot; i < 7 + rot; i++) {
    float ri = rad_for_hex(i);
    c = c.mut(0.03);
    vbo.push_vertex(x + r * cos(ri), y + r * sin(ri), c);
  }

  vbo.draw(GL_TRIANGLE_FAN);
}

seed s(0.2, 0.2, 0.3, 0.4, 0.2, 0.5);

void updateGeometry(ShaderProgram& program) {
  // hex_at(0, 0, 10, { 1, 1, 1 });

  for (int i = 0; i < 1; i++) {
    triangle_at(program, s);
    s.mutate(true);
  }
}

void game_loop(SDL_Window* window) {
  std::vector<float> vertices;

  // TODO - proc tohle nefunguje?
  // glEnable(GL_POLYGON_SMOOTH | GL_MULTISAMPLE);

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  ShaderProgram program{"vertex.glsl", "fragment.glsl"};
  std::cerr << glGetError() << std::endl;

  std::cerr << glGetError() << std::endl;

  mat grid{10, 10};

  for (int i = 2; i < 8; i++) {
    grid(3, i) = HexType::Wall;
  }

  grid.move_player(5, 4);

  SDL_Event windowEvent;
  while (true) {
    // std::cerr << glGetError() << std::endl;

    if (SDL_PollEvent(&windowEvent)) {
      if (windowEvent.type == SDL_QUIT ||
          (windowEvent.type == SDL_KEYUP &&
           windowEvent.key.keysym.sym == SDLK_ESCAPE))
        break;
    }

    glClearColor(0.3f, 0.2f, 0.3f, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    int count = 3;
    int player_dx, player_dy;
    do {
      float p_dx = (rand() / (float)RAND_MAX);
      float p_dy = (rand() / (float)RAND_MAX);

      if (p_dx > 0.7)
        player_dx = 1;
      else if (p_dx > 0.35)
        player_dx = -1;
      else
        player_dx = 0;

      if (p_dy > 0.7)
        player_dy = 1;
      else if (p_dy > 0.35)
        player_dy = -1;
      else
        player_dy = 0;

    } while (!grid.step_player(player_dx, player_dy) && count--);

    float r = 0.1;
    float height = 2 * r;
    float width = cos(30 * M_PI / 180) * r * 2;
    float height_offset = r + sin(30 * M_PI / 180) * r;

    color c_wall = {0.1f, 0.03f, 0.1f};
    color c_empty = {0.4f, 0.2f, 0.4f};
    color c_player = {0.7f, 0.4f, 0.7f};

    float start_x = -0.8;
    float start_y = 0.8;

    for (int i = 0; i < grid.m; i++) {
      for (int j = 0; j < grid.n; j++) {
        float x = start_x + j * width;
        float y = start_y - i * height_offset;

        if (i % 2 == 0) {
          x += width / 2;
        }

        color c;

        switch (grid(i, j)) {
          case HexType::Empty:
            c = c_empty;
            break;
          case HexType::Wall:
            c = c_wall;
            break;
          case HexType::Player:
            c = c_player;
            break;
        }
        hex_at(program, x, y, r, c);
        c = c.mut(0.004);
      }
    }

    SDL_GL_SwapWindow(window);
  }
}

int main(int argc, char** argv) {
  // TODO - error handling
  SDL_Init(SDL_INIT_VIDEO);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

  // TODO - error handling
  SDL_Window* window = SDL_CreateWindow(
      "OpenGL", 300, 300,  // TODO - better default screen position
      800, 600, SDL_WINDOW_OPENGL);

  // TODO - error handling
  SDL_GLContext context = SDL_GL_CreateContext(window);

  gladLoadGLLoader(SDL_GL_GetProcAddress);

  game_loop(window);

  SDL_GL_DeleteContext(context);
  // SDL_Delay(1000);
  SDL_Quit();

  return 0;
}
