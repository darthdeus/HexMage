#ifndef MOB_HPP
#define MOB_HPP

#include <algorithm>
#include <assert.h>
#include <vector>
#include <numeric>
#include <cmath>
#include <random>
#include <iostream>
#include <chrono>

struct stopwatch
{
	std::chrono::time_point<std::chrono::high_resolution_clock> start_;

	stopwatch() {
		start();
	}

	void start() {
		start_ = std::chrono::high_resolution_clock::now();
	}

	int ms() const {
		auto end = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::milliseconds>(end - start_).count();
	}
};


namespace model
{
	// TODO - fuj, ale co s tim jinyho udelat?
#undef min
#undef max

	enum class HexType
	{
		Empty = 0,
		Wall
	};

  struct Coord;
  struct Cube;

  struct Cube {
    int x;
    int y;
    int z;

    Cube(): x(0), y(0), z(0) {}
    Cube(const Coord& axial);
    Cube(int x, int y, int z): x(x), y(y), z(z) {}

    operator Coord() const;
    Cube abs() const;
    int min() const;
    int max() const;
  };

  struct Coord {
    int x;
    int y;

    Coord(): x(0), y(0) {}
    Coord(const Cube& cube);
    Coord(int x, int y): x(x), y(y) {}

    operator Cube() const;
    Coord abs() const;
    int min() const;
    int max() const;

    int distance() const;
  };

  Coord operator+(const Coord& lhs, const Coord& rhs);
  Coord operator-(const Coord& lhs, const Coord& rhs);
  bool operator==(const Coord& lhs, const Coord& rhs);
  std::ostream& operator<<(std::ostream& os, const Coord& c);

	constexpr int ABILITY_COUNT = 6;

  struct Position {
    float x;
    float y;

    Position() : x(INFINITY), y(INFINITY) {}
    Position(float x, float y): x(x), y(y) {}

    float distance() const;
  };

  Position operator+(const Position& lhs, const Position& rhs);
  Position operator-(const Position& lhs, const Position& rhs);
  bool operator==(const Position& lhs, const Position& rhs);

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

	class Arena
	{
	public:
		std::size_t size;
    Matrix<HexType> hexes;
    Matrix<Position> positions;

		Arena(std::size_t size) : size(size), hexes(size), positions(size) {}

		bool is_valid_coord(const Coord& c) const {
      return static_cast<std::size_t>(c.abs().max()) <= size;
		}

    HexType& operator()(Coord c) { return hexes(c); }
    Position& pos(Coord c) { return positions(c); }

    Coord highlight_near(Position pos);
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
			} else {
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

		void add_mob(Mob mob) {
			mobs.push_back(mob);
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
		Arena& arena;
		PlayerInfo& info;

		GameInstance(Arena& arena, PlayerInfo& info) : arena(arena), info(info) {}

		std::vector<Mob*> start_turn() const {
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

class DummySimulation
{
public:
	void run() {
		using namespace model;

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> dis(-10, 10);
		std::uniform_int_distribution<int> cost_dis(3, 7);

		Mob::abilities_t abilities;
		for (int i = 0; i < ABILITY_COUNT; ++i) {
			abilities.emplace_back(dis(gen), dis(gen), cost_dis(gen));
		}

		std::size_t map_size = 3;

		Arena arena(map_size);
		PlayerInfo info(map_size);

		GameInstance game(arena, info);

		Mob main_mob(10, 10, abilities);
		info.add_mob(main_mob);

		constexpr int SIM_TIME = 100000000;

		// Mob& player = info.mob_at({0, 0});

		std::uniform_int_distribution<int> action_dis(0, 6);
		std::uniform_int_distribution<int> move_dis(-2, 2);

		double total_time = 0;
		std::size_t iterations = 0;
		stopwatch s;
		for (int i = 0; i < SIM_TIME; ++i) {
			auto mob_list = game.start_turn();

			for (auto mob : mob_list) {
				int roll = action_dis(gen);

				// Take a random step
				if (roll == 0) {
					mob->move(arena, {move_dis(gen), move_dis(gen)});
				} else {
					// Use a random ability
					mob->use_ability(roll - 1, Target{});
				}
			}
			iterations++;
		}
		total_time += s.ms();

		std::cout << "Simulated " << iterations << " iterations took: " << total_time << "ms, per second: " << iterations / total_time * 1000 << std::endl;
	}
};

#endif /* MOB_HPP */

