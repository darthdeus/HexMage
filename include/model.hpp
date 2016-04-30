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
	public:
		static constexpr float radius = 0.1f;
		std::size_t size;
		Matrix<HexType> hexes;
		Matrix<Position> positions;
		Matrix<Path> paths;
		std::vector<float> vertices;

		Arena(std::size_t size) : size(size), hexes(size), positions(size), paths(size) {}

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
		static void draw(glm::vec2 pos, gl::Batch& b, float percentage) {
			using namespace glm;

			float hb_x = Arena::radius / 5;
			float hb_y = Arena::radius * 0.7f;
			b.push_quad(
				pos + vec2(-hb_x, -hb_y),
				pos + vec2(hb_x, -hb_y),
				pos + vec2(hb_x, hb_y),
				pos + vec2(-hb_x, hb_y),
				{ 0, 0.5, 0, 1 }
			);

			float hb_max = 2 * hb_y * percentage;

			b.push_quad(
				pos + vec2(-hb_x, -hb_y),
				pos + vec2(hb_x, -hb_y),
				pos + vec2(hb_x, hb_max - hb_y),
				pos + vec2(-hb_x, hb_max - hb_y),
				{ 0, 1, 0, 1 }
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

