#ifndef MOB_HPP
#define MOB_HPP

#pragma once

#include <algorithm>
#include <vector>
#include <iostream>
#include <random>
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
	class Team;
	class Mob;
	class Player;
	class Arena;
	class PlayerInfo;

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

		operator glm::vec2() { return {x, y}; }
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

		T& operator()(std::size_t i, std::size_t j) { return vs[n * i + j]; }
		T& operator()(const Coord& c) { return vs[n * c.y + c.x]; }

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

	inline std::ostream& operator<<(std::ostream& os, const Color& c) {
		return os << c.r << "," << c.g << "," << c.b << "," << c.a;
	}

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

	class PlayerInfo
	{
	public:
		std::vector<Mob> mobs;
		std::vector<Team> teams;
		std::size_t size;

		PlayerInfo(std::size_t size);

		Mob& add_mob(Mob mob);
		Mob& mob_at(Coord c);
		int register_team(Player& player);
		Team& team_id(int id);
	};


	class Arena
	{
		gl::Batch b;

		gl::VAO vao;
		gl::VBO vbo;

		gl::Shader shader{ "vertex.glsl", "fragment.glsl" };
	public:

		static constexpr float radius = 0.1f;
		std::size_t size;
		Matrix<HexType> hexes;
		Matrix<Position> positions;
		Matrix<Path> paths;
		std::vector<float> vertices;

		explicit Arena(std::size_t size);
		bool is_valid_coord(const Coord& c) const;
		HexType& operator()(Coord c);
		Position& pos(Coord c);
		Coord hex_near(Position pos);

		void dijkstra(Coord start);
		void regenerate_geometry();
		void draw_vertices();
		void set_projection(const glm::mat4& projection);
		void paint_hex(Position pos, float radius, Color color);
		void paint_healthbar(glm::vec2 pos, float hp, float ap);
		void paint_mob(PlayerInfo& info, const Mob& mob);
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
		int team;

		Mob(int max_hp, int max_ap, abilities_t abilities, int team);
		bool use_ability(int index, Target target);
		void move(Arena& arena, Coord d);
	};

	class Hex
	{
	public:
		HexType type;
	};

	class Player
	{
	public:
		virtual ~Player() = default;
	};

	class Team
	{
		int number = -1;
		Player& player_;
		std::vector<Mob*> mobs_;
	public:
		glm::vec3 color;

		Team(int number, Player& player)
			: number(number),
			  player_(player)
		{
			using namespace std;
			random_device rd;
			mt19937 gen(rd());
			uniform_real_distribution<float> dis(0.0f, 1.0f);

			color = { dis(gen), dis(gen), dis(gen) };
		}

		void add_mob(Mob& mob) { mobs_.push_back(&mob); }
	};

	class UserPlayer : public Player
	{
		
	};

	class AIPlayer : public Player
	{
		
	};

	class Turn
	{
		std::vector<Mob*> mobs_;
		std::vector<Mob*>::iterator current_;
	public:
		Turn() = default;
		explicit Turn(std::vector<Mob>& mobs);

		bool is_done() const { return current_ == mobs_.end(); }
		Mob* next();
	};

	class GameInstance
	{
	public:
		Arena arena;
		PlayerInfo info;
		std::size_t size;

		GameInstance(std::size_t size) : arena(size), info(size), size(size) {}

		Turn start_turn();
	};

	class TurnManager
	{
		PlayerInfo& info_;
	public:
		Turn current_turn;

		explicit TurnManager(PlayerInfo& info):
			info_(info) {}
		// TODO 
	};
}


#endif /* MOB_HPP */

