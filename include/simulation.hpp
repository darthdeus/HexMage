#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#pragma once

#include <vector>
#include <random>
#include <model.hpp>
#include <stopwatch.hpp>
#include <generator.hpp>
#include <format.h>

namespace simulation
{
	extern std::vector<std::string> profiling_results;
	void dummy_profiling();

	constexpr int ABILITY_COUNT = 6;

	class DummySimulation
	{
	public:
		void run();
	};

}


#endif
