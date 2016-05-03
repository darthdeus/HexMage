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

}
