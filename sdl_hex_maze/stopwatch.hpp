#pragma once

#include <chrono>

struct stopwatch
{
	std::chrono::time_point<std::chrono::high_resolution_clock> start_;

	stopwatch() {
		start();
	}

	void start() {
		start_ = std::chrono::high_resolution_clock::now();
	}

	int ms() {
		auto end = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::milliseconds>(end - start_).count();
	}
};