#pragma once

#include <atomic>

struct ApplicationClock
{
protected:
	double applicationStartTime = 0.0;

public:
	std::atomic<double> time = 0.0;
	std::atomic<double> deltaTime = 0.0;

	ApplicationClock();
	~ApplicationClock() {}

	void Tick();
	static double SecondsSinceEpoch();
	inline double TimeSinceAppStart() const { return SecondsSinceEpoch() - applicationStartTime; }
	inline double TimeSinceLastTick() const { return TimeSinceAppStart() - time; }
};
