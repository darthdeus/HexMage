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

  Coord operator+(const Coord& lhs, const Coord& rhs) {
    return {lhs.x + rhs.x, lhs.y + rhs.y};
  }

  Coord operator-(const Coord& lhs, const Coord& rhs) {
    return {lhs.x - rhs.x, lhs.y - rhs.y};
  }

  bool operator==(const Coord& lhs, const Coord& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }

  std::ostream& operator<<(std::ostream& os, const Coord& c) {
    return os << "(" << c.x << "," << c.y << ")";
  }
}
