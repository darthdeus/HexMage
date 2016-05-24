#ifndef MAP_GEOMETRY_HPP
#define MAP_GEOMETRY_HPP

#include <sim.hpp>
#include <gl_utils.hpp>

using Position = glm::vec2;

glm::vec4 color_for_type(sim::HexType type);

class MapGeometry {
  sim::Game& game_;

  gl::Batch b;

  gl::VAO vao;
  gl::VBO vbo;

  gl::Shader shader{"res/vertex.glsl", "res/fragment.glsl"};

 public:
  static constexpr float radius = 0.1f;

  sim::Matrix<Position> positions;

  std::vector<float> vertices;

  explicit MapGeometry(sim::Game& game);

  Position& pos(sim::Coord c);
  sim::Coord hex_near(Position pos);

  void regenerate_geometry(boost::optional<int> current_ap = boost::none);
  void draw_vertices();

  void set_projection(const glm::mat4& projection);
  void paint_hex(Position pos, float radius, glm::vec4 color);
  void paint_healthbar(Position pos, float hp, float ap);
  void paint_mob(sim::Mob& mob);
};

#endif /* MAP_GEOMETRY_HPP */
