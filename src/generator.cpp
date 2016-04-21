#include <random>
#include <model.hpp>
#include <simulation.hpp>

namespace generator
{
	model::Mob random_mob() {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> dis(-10, 10);
		std::uniform_int_distribution<int> cost_dis(3, 7);

		model::Mob::abilities_t abilities;
		for (int i = 0; i < simulation::ABILITY_COUNT; ++i) {
			abilities.emplace_back(dis(gen), dis(gen), cost_dis(gen));
		}

		return model::Mob{ 10, 10, abilities };
	}
}
