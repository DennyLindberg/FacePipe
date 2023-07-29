#pragma once

#include <atomic>

struct ApplicationClock
{
private:
	uint64_t sdl_ms_previous = 0;
	uint64_t sdl_ms_current = 0;
	double lastTickTime = 0.0;

public:
	std::atomic<double> time = 0.0;
	std::atomic<double> deltaTime = 0.0;

	ApplicationClock();
	~ApplicationClock() {}

	void Tick();
	static double SystemTime();
	double TimeSinceLastTick() const { return SystemTime() - lastTickTime; }
};
