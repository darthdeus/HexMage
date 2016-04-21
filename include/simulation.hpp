#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#pragma once

#include <vector>
#include <model.hpp>
#include <generator.hpp>

namespace simulation
{
	extern std::vector<std::string> profiling_results;
	void dummy_profiling();

	constexpr int ABILITY_COUNT = 6;

	class DummySimulation
	{
	public:
		void run() {
			using namespace model;

			std::random_device rd;
			std::mt19937 gen(rd());

			std::size_t map_size = 3;

			GameInstance game(map_size);

			Arena& arena = game.arena;
			PlayerInfo& info = game.info;

			info.add_mob(generator::random_mob());

			constexpr int SIM_TIME = 10000000;

			std::uniform_int_distribution<int> action_dis(0, 6);
			std::uniform_int_distribution<int> move_dis(-2, 2);

			double total_time = 0;
			std::size_t iterations = 0;

			Stopwatch s;
			for (int i = 0; i < SIM_TIME; ++i) {
				auto mob_list = game.start_turn();

				for (auto mob : mob_list) {
					int roll = action_dis(gen);

					// Take a random step
					if (roll == 0) {
						mob->move(arena, { move_dis(gen), move_dis(gen) });
					}
					else {
						// Use a random ability
						mob->use_ability(roll - 1, Target{});
					}
				}
				iterations++;
			}
			total_time += s.ms();

			std::cout << "Simulated " << iterations << " iterations took: " << total_time << "ms, per second: " << (int)(iterations / total_time * 1000)
				<< "\t iteration " << ((float)total_time) / iterations << "ms" << std::endl;
		}
	};

}


#endif
