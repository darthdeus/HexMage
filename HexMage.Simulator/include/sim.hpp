#ifndef HEXMAGE_SIM_SIM_HPP
#define HEXMAGE_SIM_SIM_HPP

#include <iostream>
#include <vector>
#include <random>

#include <format.h>
#include "utils.hpp"
#include "glm/glm.hpp"

namespace sim {
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


	class Ability
	{
	public:
		int d_hp;
		int d_ap;
		int cost;
		int range = 5;

		Ability(int d_hp, int d_ap, int cost) : d_hp(d_hp), d_ap(d_ap), cost(cost) {}
	};


	class Target {
		Mob& mob_;
	public:
		Target(Mob &);
	};

	class Path {};

	class Map {};
	class Players {};

	class Pathfinder {
	public:
		Path path_to(Target target);
		void move_as_far_as_possible(Mob &, Path &);
	};

	class TurnManager {
	public:
		TurnManager(Players &);
		bool is_turn_done() const;
		void start_next_turn();
		Mob &current_mob() const;
	};

	class UsableAbility {
	public:
		void use();
	};

	class Game {
    Players players_;
    Pathfinder pathfinder_;
    TurnManager turn_manager_;

	public:
		Players &players();
		Pathfinder &pathfinder();
		TurnManager &turn_manager();

		Mob &add_mob(Mob mob);

		bool is_finished() const;

    std::size_t size() const;
    Index<Team> add_team();

		// 1. jake schopnoasti muzu pouzit - sebe
		// 1b. jake schopnoasti muzu pouzit na policko - sebe, hrace, cesty
		std::vector<Ability> usable_abilities(Mob &);
		std::vector<UsableAbility> usable_abilities(Mob &, Target, Players &,
			Pathfinder &);

		// 2. na koho muzu utocit - sebe, hrace, cesty
		std::vector<Target> possible_targets(Mob &, Players &, Pathfinder &);

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

		int hp;
		int ap;

		abilities_t abilities;
		glm::vec2 c;
		Index<Team> team;

		Mob(int max_hp, int max_ap, abilities_t abilities, Index<Team> team);
	};

	class Team
	{
		int number = -1;
		std::vector<Mob*> mobs_;
	public:
		glm::vec3 color;

		Team(int number);
		void add_mob(Mob& mob) { mobs_.push_back(&mob); }
		inline int id() const { return number; }
	};

	inline bool operator==(const Team& lhs, const Team& rhs) {
		return lhs.id() == rhs.id();
	}

	inline bool operator!=(const Team& lhs, const Team& rhs) {
		return lhs.id() != rhs.id();
	}
};

#endif /* SIM_HPP */
