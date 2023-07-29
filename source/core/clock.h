#pragma once

#include <atomic>

struct ApplicationClock
{
public:
	std::atomic<double> time = 0.0;
	std::atomic<double> deltaTime = 0.0;

	ApplicationClock();
	~ApplicationClock() {}

	void Tick();
	static double SecondsSinceEpoch();
	double TimeSinceLastTick() const { return SecondsSinceEpoch() - time; }
};
