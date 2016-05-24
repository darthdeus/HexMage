#include <map_geometry.hpp>

glm::vec4 color_for_type(sim::HexType type) {
  switch (type) {
    case sim::HexType::Empty:
      return{ 0.4f, 0.2f, 0.4f, 1.0f };
    case sim::HexType::Wall:
      return{ 0.1f, 0.03f, 0.1f, 1.0f };
    case sim::HexType::Player:
      return{ 0.7f, 0.4f, 0.7f, 1.0f };
    default:
      throw "invalid hex type";
  }
}


MapGeometry::MapGeometry(sim::Game& game)
    : game_(game), positions(game.size(), game.size()) {
  gl::Vertex::setup_attributes();
  shader.set("projection", glm::mat4(1.0f));
}

Position& MapGeometry::pos(sim::Coord c) { return positions(c); }

sim::Coord MapGeometry::hex_near(Position rel_pos) {
  sim::Coord closest;
  float min = INFINITY;

  for (std::size_t i = 0; i < positions.m; ++i) {
    for (std::size_t j = 0; j < positions.n; ++j) {
      auto pos = positions(i, j);

      float distance = sim::hex_distance(pos - rel_pos);

      if (distance < min) {
        closest = {static_cast<int>(j), static_cast<int>(i)};
        min = distance;
      }
    }
  }

  return closest;
}


void MapGeometry::regenerate_geometry(boost::optional<int> current_ap) {
  float start_x = -0.5f;
  float start_y = -0.5f;

  float width = static_cast<float>(cos(30 * M_PI / 180) * radius * 2);
  float height_offset =
      static_cast<float>(radius + sin(30 * M_PI / 180) * radius);

  b.clear();

  auto&& pathfinder = game_.pathfinder();

  int isize = static_cast<int>(size);
  for (int row = 0; row < isize; ++row) {
    for (int col = 0; col < isize; ++col) {
      float draw_x = start_x;
      float draw_y = start_y;

      // axial q-change
      draw_x += col * width;
      // axial r-change
      draw_x += row * (width / 2);
      draw_y += row * height_offset;

      pos({col, row}) = {draw_x, draw_y};

      auto type = game_.map()({col, row});
      glm::vec4 c = color_for_type(type);
      auto path = pathfinder({col, row});

      if (path.distance < 0) {
        if (path.source) {
          fmt::printf("Distance negative %i at %i,%i, source %i,%i\n",
                      path.distance, row, col, path.source->x, path.source->y);
        } else {
          fmt::printf("Distance negative %i at %i,%i, no source\n",
                      path.distance, row, col);
        }
      }

      if (current_ap) {
        if (type == sim::HexType::Empty && path.distance > 0) {
          // distance = 1 -> 0.3
          // distance = ap -> 0
          float change = (*current_ap + 1 - path.distance) * 0.06f;
          if (change > 0) {
            c += glm::vec4(change);
          }
        }
      }

      auto pos = this->pos({col, row});
      b.push_hex({pos.x, pos.y, 0}, c, radius);
      // hex_at(vertices, pos, radius, c);
      c += glm::vec4(0.004f);
    }
  }
}

void MapGeometry::draw_vertices() {
  shader.use();
  vao.bind();
  vbo.bind();
  b.draw_arrays();

  // glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(),
  // vertices.data(), GL_STATIC_DRAW);
  // glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()) / 6);
}

void MapGeometry::set_projection(const glm::mat4& projection) {
  shader.set("projection", projection);
}

void MapGeometry::paint_hex(Position pos, float radius, glm::vec4 color) {
  vao.bind();
  vbo.bind();
  shader.use();
  gl::Batch b;
  b.push_hex(pos, color, radius);
  b.draw_arrays();
}

void MapGeometry::paint_healthbar(glm::vec2 pos, float hp, float ap) {
  using namespace glm;
  using namespace std;

  vao.bind();
  vbo.bind();
  shader.use();

  gl::Batch b;
  float width = MapGeometry::radius / 5 * 2;
  float height = MapGeometry::radius * 0.7f * 2;

  float hp_max = height * hp;
  float ap_max = height * ap;

  b.push_quad_bot_left({pos.x - width, pos.y - height / 2}, width, height, 0,
                       {0, 0.5, 0, 1});
  b.push_quad_bot_left({pos.x - width, pos.y - height / 2}, width, hp_max, 0,
                       {0, 1, 0, 1});

  b.push_quad_bot_left({pos.x, pos.y - height / 2}, width, height, 0,
                       {0.5, 0.5, 0, 1});
  b.push_quad_bot_left({pos.x, pos.y - height / 2}, width, ap_max, 0,
                       {1, 1, 0, 1});

  b.draw_arrays();
}

void MapGeometry::paint_mob(sim::Mob& mob) {
  auto turn_manager = game_.turn_manager();

  auto p = pos(mob.c);
  // auto c = mob.team->color;
  auto c = glm::vec3(0.3, 0.7, 0.3);

  if (mob == turn_manager.current_mob()) {
    c += glm::vec3(0.3f);
  }
  paint_hex(p, radius, glm::vec4(c, 1.0f));
  paint_healthbar(p, (float)mob.hp / mob.max_hp, (float)mob.ap / mob.max_ap);
}

