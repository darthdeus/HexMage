#include <random>
#include <model.hpp>
#include <simulation.hpp>

namespace generator
{
	model::Mob random_mob(int team, std::size_t size) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> dis(1, 10);
		std::uniform_int_distribution<int> cost_dis(3, 7);
		std::uniform_int_distribution<int> pos_dis(0, (int)size);

		model::Mob::abilities_t abilities;
		for (int i = 0; i < simulation::ABILITY_COUNT; ++i) {
			abilities.emplace_back(dis(gen), dis(gen), cost_dis(gen));
		}

		auto mob =  model::Mob{ 10, 10, abilities, team};
		mob.c = { pos_dis(gen), pos_dis(gen) };
		return mob;
	}
}
