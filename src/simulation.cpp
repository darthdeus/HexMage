#include <simulation.hpp>
#include <format.h>

namespace simulation
{

	std::vector<std::string> profiling_results;

	void dummy_profiling() {
		using namespace model;
		profiling_results.clear();

		GameInstance g(30);
		g.info.add_mob(generator::random_mob());
		g.info.add_mob(generator::random_mob());
		g.info.add_mob(generator::random_mob());
		g.info.add_mob(generator::random_mob());
		g.info.add_mob(generator::random_mob());
		g.info.add_mob(generator::random_mob());
		g.info.add_mob(generator::random_mob());
		g.info.add_mob(generator::random_mob());
		g.info.add_mob(generator::random_mob());
		g.info.add_mob(generator::random_mob());

		Stopwatch ss;

		//int iterations = 100000;

		//std::size_t total_size = 0;

		//ss.start();
		//for (int i = 0; i < iterations; i++) {
		//	auto r = g;
		//	total_size += r.size;
		//}

		std::string str;
		//str = fmt::sprintf("GameInstance copy iterations %d took %dms\t%fus", iterations, ss.ms(), ((float)ss.ms()) / iterations * 1000);
		str = fmt::sprintf("!!!!!!!!! GAME INSTANCE COPYING DISABLED, BENCHMARK NOT RUN !!!!!!!!!");
		profiling_results.push_back(str);

		PlayerInfo ifo = g.info;

		std::size_t total_mobs = 0;
		ss.start();

		int info_iterations = 1000000;
		for (int i = 0; i < info_iterations; ++i) {
			auto copy = ifo;
			total_mobs += copy.mobs.size();
		}

		str = fmt::sprintf("PlayerInfo copy iterations %d took %dms\t%fus", info_iterations, ss.ms(), ((float)ss.ms()) / info_iterations * 1000);
		profiling_results.push_back(str);

		ss.start();
		DummySimulation sim;
		sim.run();

		str = fmt::sprintf("DummySimulation took %dms", ss.ms());
		profiling_results.push_back(str);
	}

	void DummySimulation::run()
	{
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

			std::cerr << "!!! SIMULATION TEMPORARILY DISABLED !!!" << std::endl;
			//for (auto mob : mob_list) {
			//	int roll = action_dis(gen);

			//	// Take a random step
			//	if (roll == 0) {
			//		mob->move(arena, {move_dis(gen), move_dis(gen)});
			//	} else {
			//		// Use a random ability
			//		mob->use_ability(roll - 1, Target{});
			//	}
			//}
			iterations++;
		}
		total_time += s.ms();

		fmt::printf("Simulated %d iterations, took: %fms, per second: %i\titeration: %fms\n",
		            iterations,
		            total_time,
		            (int)(iterations / total_time * 1000),
		            ((float)total_time) / iterations);
	}
}
