#ifndef HEXMAGE_SIM_SIM_HPP
#define HEXMAGE_SIM_SIM_HPP

#include <iostream>
#include <vector>
#include <random>

#include <boost/optional.hpp>
#include <format.h>
#include "utils.hpp"
#include "glm/glm.hpp"

namespace sim
{
	constexpr int ABILITY_COUNT = 6;

	class Mob;
	class Ability;
	class Path;
	class Target;
	class Map;
	class Pathfinder;
	class TurnManager;
	class Ability;
	class UsableAbility;
	class Game;
	class Team;

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
		T& operator()(const glm::vec2& c) { return vs[n * c.y + c.x]; }

	private:
	}; /* column-major/opengl: vs[i + m * j], row-major/c++: vs[n * i + j] */

	class Ability
	{
	public:
		int d_hp;
		int d_ap;
		int cost;
		int range = 5;

		Ability(int d_hp, int d_ap, int cost);
	};

	class Target
	{
		Mob& mob_;
	public:
		Target(Mob&);
	};

	class Map
	{
		Matrix<glm::vec2> hexes_;
		std::size_t size_;
	public:
		explicit Map(std::size_t size);

		Matrix<glm::vec2> hexes();
	};

	class MobManager
	{
		std::vector<Mob> mobs_;
		std::vector<Team> teams_;
	public:
		Index<Mob> add_mob(Mob mob);
		Index<Team> add_team();
		std::vector<Mob>& mobs();
		std::vector<Team>& teams();
	};

	enum class VertexState { Open, Closed, Unvisited };

	class Path
	{
	public:
		boost::optional<glm::vec2> source;
		VertexState state;
		int distance;
	};

	class Pathfinder
	{
		Matrix<Path> paths_;
	public:
		explicit Pathfinder(std::size_t size);

		Matrix<Path>& paths();

		Path path_to(Target target);
		void move_as_far_as_possible(Mob&, Path&);
		void pathfind_from(glm::vec2 source);
		std::size_t distance(glm::vec2 t1, glm::vec2 t2);
		void update(glm::vec2 start, Map& map, MobManager& mob_manager);
	};

	class TurnManager
	{
		MobManager& mob_manager_;
		std::vector<Mob*> turn_order_;
		std::size_t current_ = 0;
	public:
		explicit TurnManager(MobManager&);
		bool is_turn_done() const;
		void start_next_turn();
		Mob& current_mob() const;
		bool move_next();
	};

	class UsableAbility
	{
	public:
		void use();
	};

	class Game
	{
		Map map_;
		MobManager mob_manager_;
		Pathfinder pathfinder_;
		TurnManager turn_manager_;
		std::size_t size_;

	public:
		explicit Game(std::size_t s);

		MobManager& players();
		Pathfinder& pathfinder();
		TurnManager& turn_manager();

		Index<Mob> add_mob(Mob mob);
		bool is_finished();

		std::size_t size() const;
		Index<Team> add_team();

		// 1. jake schopnoasti muzu pouzit - sebe
		// 1b. jake schopnoasti muzu pouzit na policko - sebe, hrace, cesty
		std::vector<Ability> usable_abilities(Mob&) const;
		std::vector<UsableAbility> usable_abilities(Mob&, Target, MobManager&,
		                                            Pathfinder&);

		// 2. na koho muzu utocit - sebe, hrace, cesty
		std::vector<Target> possible_targets(Mob&, MobManager&, Pathfinder&);

		//
		// 3. kdo je na tahu - tahovatko

		//
		// 4. kdo updatuje stav - hrace, sebe, mapu
		// 5. kdo resi cesty - hrace, sebe, mapu
	};

	class Mob
	{
	public:
		using abilities_t = std::vector<Ability>;
		const int max_hp;
		const int max_ap;

		int hp = 0;
		int ap = 0;

		abilities_t abilities;
		glm::vec2 c;
		Index<Team> team;

		Mob(int max_hp, int max_ap, abilities_t abilities, Index<Team> team);
	};

	class Team
	{
		int number = -1;
		std::vector<Mob*> mobs_;
		glm::vec3 color_;
	public:

		Team(int number);

		void add_mob(Mob& mob);
		int id() const;
		glm::vec3& color();
		std::vector<Mob*> mobs();
	};

	inline bool operator==(const Team& lhs, const Team& rhs) {
		return lhs.id() == rhs.id();
	}

	inline bool operator!=(const Team& lhs, const Team& rhs) {
		return lhs.id() != rhs.id();
	}
};

#endif /* SIM_HPP */
