#pragma once

#include <atomic>
#include <chrono>
#include <functional>

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

struct ScopedPerfCounterSeconds
{
	std::function<void(double dur)> fun;
	std::chrono::high_resolution_clock::time_point start;

	ScopedPerfCounterSeconds(std::function<void(double dur)> callback)
		: fun(callback), start(std::chrono::high_resolution_clock::now())
	{}

	~ScopedPerfCounterSeconds()
	{
		if (fun)
		{
			std::chrono::duration<double, std::nano> ms_double = std::chrono::high_resolution_clock::now() - start;
			fun(ms_double.count() * 0.000000001);
		}
	}
};
