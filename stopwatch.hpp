#ifndef STOPWATCH_HPP
#define STOPWATCH_HPP

#include <chrono>

struct Stopwatch
{
	std::chrono::time_point<std::chrono::high_resolution_clock> start_;

	Stopwatch() {
		start();
	}

	void start() {
		start_ = std::chrono::high_resolution_clock::now();
	}

	int64_t ms() const {
		auto end = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::milliseconds>(end - start_).count();
	}

	float ms_f() const {
		auto end = std::chrono::high_resolution_clock::now();
		auto val = std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count();
		return (float)val / 1000.0f;
	}
};

#endif