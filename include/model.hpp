#ifndef MOB_HPP
#define MOB_HPP

#pragma once

#include <algorithm>
#include <vector>
#include <iostream>
#include <gl_utils.hpp>
#include <boost/optional.hpp>

namespace model
{
	enum class HexType
	{
		Empty = 0,
		Wall,
		Player
	};

	struct Coord;
	struct Cube;

	struct Cube
	{
		int x;
		int y;
		int z;

		Cube() : x(0), y(0), z(0) {}

		Cube(const Coord& axial);

		Cube(int x, int y, int z) : x(x), y(y), z(z) {}

		operator Coord() const;
		Cube abs() const;
		int min() const;
		int max() const;
	};

	struct Coord
	{
		int x;
		int y;

		Coord() : x(0), y(0) {}

		Coord(const Cube& cube);

		Coord(int x, int y) : x(x), y(y) {}

		operator Cube() const;
		Coord abs() const;
		int min() const;
		int max() const;

		int distance() const;
	};

	Coord operator+(const Coord& lhs, const Coord& rhs);
	Coord operator-(const Coord& lhs, const Coord& rhs);
	bool operator==(const Coord& lhs, const Coord& rhs);

	inline bool operator!=(const Coord& lhs, const Coord& rhs)
	{
		return !(lhs == rhs);
	}

	std::ostream& operator<<(std::ostream& os, const Coord& c);

	constexpr int ABILITY_COUNT = 6;

	struct Position
	{
		float x;
		float y;

		Position() : x(INFINITY), y(INFINITY) {}

		Position(const glm::vec2& v) : x(v.x), y(v.y) {}

		Position(float x, float y) : x(x), y(y) {}

		float distance() const;
		Position operator-() const;
		operator glm::vec2() const;

		Position abs() const;
		float min() const;
		float max() const;
		Position& operator+=(const Position& position);
		Position& operator-=(const Position& position);

		operator glm::vec2()
		{
			return {x, y};
		}
	};

	Position operator+(const Position& lhs, const Position& rhs);
	Position operator-(const Position& lhs, const Position& rhs);
	bool operator==(const Position& lhs, const Position& rhs);
	std::ostream& operator<<(std::ostream& os, const Position& p);

	Position mouse2gl(int x, int y);

	// TODO - update this to a proper container
	template <typename T>
	struct Matrix
	{
		std::size_t m;
		std::size_t n;
		std::vector<T> vs;

		Matrix(std::size_t m, std::size_t n) : m(m), n(n), vs(m * n) {}

		// Create a matrix that can contain data for a hex with radius `size`
		Matrix(std::size_t size) : Matrix(size * 2 + 1, size * 2 + 1) {}

		T& operator()(std::size_t i, std::size_t j)
		{
			return vs[n * i + j];
		}

		T& operator()(const Coord& c)
		{
			return vs[n * c.y + c.x];
		}

	private:
	}; /* column-major/opengl: vs[i + m * j], row-major/c++: vs[n * i + j] */

	struct Color
	{
		float r, g, b, a;

		Color() : r(0), g(0), b(0), a(0) {}
		Color(float r, float g, float b) : r(r), g(g), b(b), a(1) {}
		Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

		Color mut(float d) const { return {r + d, g + d, b + d, a}; }
		operator glm::vec4() const { return {r, g, b, a}; }
	};

	model::Color color_for_type(HexType type);

	class Ability
	{
	public:
		int d_hp;
		int d_ap;
		int cost;

		Ability(int d_hp, int d_ap, int cost) : d_hp(d_hp), d_ap(d_ap), cost(cost) {}
	};

	class Target
	{
	public:
		int x;
		int y;
	};

	enum class VertexState { Unvisited, Open, Closed };

	class Path
	{
	public:
		boost::optional<Coord> source;
		VertexState state;
		int distance;
	};

	class Arena
	{
		gl::Batch b;
	public:
		gl::VAO vao;
		gl::VBO vbo;

		static constexpr float radius = 0.1f;
		std::size_t size;
		Matrix<HexType> hexes;
		Matrix<Position> positions;
		Matrix<Path> paths;
		std::vector<float> vertices;
		gl::Shader shader{ "vertex.glsl", "fragment.glsl" };

		explicit Arena(std::size_t size);

		bool is_valid_coord(const Coord& c) const {
			return static_cast<std::size_t>(c.abs().max()) < size && c.min() >= 0;
		}

		HexType& operator()(Coord c) {
			return hexes(c);
		}

		Position& pos(Coord c) {
			return positions(c);
		}

		Coord hex_near(Position pos);

		void dijkstra(Coord start);
		void regenerate_geometry();
		void draw_vertices();
	};

	class Healthbar
	{
	public:
		static void draw(glm::vec2 pos, gl::Batch& b, float hp, float ap) {
			using namespace glm;
			using namespace std;

			float width = Arena::radius / 5 * 2;
			float height = Arena::radius * 0.7f * 2;

			float hp_max = height * hp;
			float ap_max = height * ap;

			b.push_quad_bot_left(
				{pos.x - width, pos.y - height / 2},
				width, height, 0, {0, 0.5, 0, 1}
			);
			b.push_quad_bot_left(
				{pos.x - width, pos.y - height / 2},
				width, hp_max, 0, {0, 1, 0, 1}
			);

			b.push_quad_bot_left(
				{pos.x, pos.y - height / 2},
				width, height, 0, {0.5, 0.5, 0, 1}
			);
			b.push_quad_bot_left(
				{pos.x, pos.y - height / 2},
				width, ap_max, 0, {1, 1, 0, 1}
			);
		}
	};

	class Mob
	{
	public:
		using abilities_t = std::vector<Ability>;
		const int max_hp;
		const int max_ap;

		int hp;
		int ap;

		abilities_t abilities;

		Coord c;

		Mob(int max_hp, int max_ap, abilities_t abilities)
			: max_hp(max_hp),
			max_ap(max_ap),
			hp(max_hp),
			ap(max_ap),
			abilities(abilities) {
			assert(abilities.size() == ABILITY_COUNT);
		}

		bool use_ability(int index, Target target) {
			assert(index <= abilities.size());

			auto& ability = abilities[index];
			if (ap >= ability.cost) {
				return true;
			}
			else {
				return false;
			}
		}

		void move(Arena& arena, Coord d) {
			if (arena.is_valid_coord(c + d)) {
				c = c + d;
				ap -= d.distance(); // TODO - better calculation
			}
		}
	};

	class Hex
	{
	public:
		HexType type;
	};

	class PlayerInfo
	{
	public:
		std::vector<Mob> mobs;
		std::size_t size;

		PlayerInfo(std::size_t size) : size(size) {}

		Mob& add_mob(Mob mob) {
			mobs.push_back(mob);
			return mobs.back();
		}

		Mob& mob_at(Coord c) {
			for (auto& m : mobs) {
				if (m.c == c) {
					return m;
				}
			}

			std::cerr << "Mob not found at " << c << std::endl;
			throw "Mob not found";
		}
	};

	class GameInstance
	{
	public:
		Arena arena;
		PlayerInfo info;
		std::size_t size;

		GameInstance(std::size_t size) : arena(size), info(size), size(size) {}

		std::vector<Mob*> start_turn() {
			std::vector<Mob*> mobs;
			for (auto& mob : info.mobs) {
				mob.ap = std::min(mob.ap, mob.ap + mob.max_ap);
				mobs.push_back(&mob);
			}

			std::sort(mobs.begin(), mobs.end(), [](Mob* m1, Mob* m2) {
				return m1->ap < m2->ap;
			});

			return mobs;
		}
	};
}


#endif /* MOB_HPP */

