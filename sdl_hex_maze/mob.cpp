#include <iostream>
#include "mob.hpp"

namespace model {
  Cube::Cube(const Coord& axial) : x(axial.x), y(-axial.x - axial.y), z(axial.y) {}
  Cube::operator Coord() const { return {*this}; }
  Cube Cube::abs() const { return {std::abs(x), std::abs(y), std::abs(z)}; }
  int Cube::min() const { return std::min(x, std::min(y, z)); }
  int Cube::max() const { return std::max(x, std::max(y, z)); }

  Coord::Coord(const Cube& cube) : x(cube.x), y(cube.z) {}
  Coord::operator Cube() const { return {*this}; }
  Coord Coord::abs() const { return {std::abs(x), std::abs(y)}; }
  int Coord::min() const { return std::min(x, y); }
  int Coord::max() const { return std::max(x, y); }
  int Coord::distance() const { return Cube(*this).abs().max(); }

  Coord operator+(const Coord& lhs, const Coord& rhs) { return {lhs.x + rhs.x, lhs.y + rhs.y}; }
  Coord operator-(const Coord& lhs, const Coord& rhs) { return {lhs.x - rhs.x, lhs.y - rhs.y}; }
  bool operator==(const Coord& lhs, const Coord& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }

  Position operator+(const Position& lhs, const Position& rhs) { return {lhs.x + rhs.x, lhs.y + rhs.y}; }
  Position operator-(const Position& lhs, const Position& rhs) { return {lhs.x - rhs.x, lhs.y - rhs.y}; }
  bool operator==(const Position& lhs, const Position& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }

  float Position::distance() const { return x * x + y * y; }

  std::ostream& operator<<(std::ostream& os, const Coord& c) {
    return os << "(" << c.x << "," << c.y << ")";
  }

  Coord Arena::highlight_near(Position rel_pos) {
    Coord closest;
    float min = INFINITY;

    for (std::size_t i = 0; i < positions.m; ++i) {
      for (std::size_t j = 0; j < positions.n;++j) {
        auto pos = positions(i, j);

        float distance = (pos - rel_pos).distance();

        if (distance < min) {
          closest = {static_cast<int>(j), static_cast<int>(i)};
          min = distance;
        }
      }
    }

    return closest;
  }
}
