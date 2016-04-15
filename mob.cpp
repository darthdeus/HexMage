#include <iostream>
#include "mob.hpp"

namespace model {
	Cube::Cube(const Coord& axial) : x(axial.x), y(-axial.x - axial.y), z(axial.y) {}
	Cube::operator Coord() const { return{ *this }; }
	Cube Cube::abs() const { return{ std::abs(x), std::abs(y), std::abs(z) }; }
	int Cube::min() const { return std::min(x, std::min(y, z)); }
	int Cube::max() const { return std::max(x, std::max(y, z)); }

	Coord::Coord(const Cube& cube) : x(cube.x), y(cube.z) {}
	Coord::operator Cube() const { return{ *this }; }
	Coord Coord::abs() const { return{ std::abs(x), std::abs(y) }; }
	int Coord::min() const { return std::min(x, y); }
	int Coord::max() const { return std::max(x, y); }
	int Coord::distance() const { return Cube(*this).abs().max(); }

	Coord operator+(const Coord& lhs, const Coord& rhs) { return{ lhs.x + rhs.x, lhs.y + rhs.y }; }
	Coord operator-(const Coord& lhs, const Coord& rhs) { return{ lhs.x - rhs.x, lhs.y - rhs.y }; }
	bool operator==(const Coord& lhs, const Coord& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }

	Position operator+(const Position& lhs, const Position& rhs) { return{ lhs.x + rhs.x, lhs.y + rhs.y }; }
	Position operator-(const Position& lhs, const Position& rhs) { return{ lhs.x - rhs.x, lhs.y - rhs.y }; }
	bool operator==(const Position& lhs, const Position& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }

	std::ostream& operator<<(std::ostream& os, const Position& p) {
		return os << "(" << p.x << "," << p.y << ")";
	}

	float Position::distance() const { return x * x + y * y; }

	Position Position::operator-() const { return{ -x, -y }; }
	Position::operator glm::vec2() const { return{ x, y }; }

	Position Position::abs() const { return{ std::abs(x), std::abs(y) }; }
	float Position::min() const { return std::min(x, y); }
	float Position::max() const { return std::max(x, y); }

	Position& Position::operator+=(const Position& p) { x += p.x; y += p.y; return *this; }
	Position& Position::operator-=(const Position& p) { x -= p.x; y -= p.y; return *this; }

	std::ostream& operator<<(std::ostream& os, const Coord& c) {
		return os << "(" << c.x << "," << c.y << ")";
	}

	Coord Arena::hex_near(Position rel_pos) {
		Coord closest;
		float min = INFINITY;

		for (std::size_t i = 0; i < positions.m; ++i) {
			for (std::size_t j = 0; j < positions.n;++j) {
				auto pos = positions(i, j);

				float distance = (pos - rel_pos).distance();

				if (distance < min) {
					closest = { static_cast<int>(j), static_cast<int>(i) };
					min = distance;
				}
			}
		}

		return closest;
	}
}

namespace generator
{
	model::Mob random_mob() {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> dis(-10, 10);
		std::uniform_int_distribution<int> cost_dis(3, 7);

		model::Mob::abilities_t abilities;
		for (int i = 0; i < ABILITY_COUNT; ++i) {
			abilities.emplace_back(dis(gen), dis(gen), cost_dis(gen));
		}

		return model::Mob{ 10, 10, abilities };
	}
}
