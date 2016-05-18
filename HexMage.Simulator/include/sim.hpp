#ifndef SIM_HPP
#define SIM_HPP

#include <iostream>
#include <vector>
#include <random>

#include <format.h>
#include "glm/glm.hpp"

// TODO - extract this into a separate header
template <typename T>
class Index
{
	std::vector<T>& v_;
	std::size_t index_;
public:
	Index(std::vector<T>& v, std::size_t index):
		v_(v), index_(index) {}

	T& get() { return v_[index_]; }
	const T& get() const { return v_[index_]; }

	T& operator*() { return get(); }
	const T& operator*() const { return get(); }

	operator T&() { return get(); }
	operator const T&() const { return get(); }

	T* operator->() { return &get(); }
	const T* operator->() const { return &get(); }

	bool operator==(const Index& rhs) const { return get() == rhs.get(); }
	bool operator!=(const Index& rhs) const { return get() != rhs.get(); }
};


namespace sim {
  class Ability;
  class Mob;
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
