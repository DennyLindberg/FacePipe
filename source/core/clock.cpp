#include "clock.h"
#include <chrono>

namespace chrono = std::chrono;

ApplicationClock::ApplicationClock()
{
	time = SecondsSinceEpoch();
	deltaTime = 0.1; // non-zero init
	lastTickTime = time-deltaTime;
}

void ApplicationClock::Tick()
{
	double newtime = SecondsSinceEpoch();
	deltaTime = newtime - lastTickTime;
	lastTickTime = time + 0.0; // atomic bullshit
	time = newtime;
}

double ApplicationClock::SecondsSinceEpoch()
{
	auto TimeSinceEpoch = chrono::system_clock::now().time_since_epoch();
	auto DurationSinceEpoch = chrono::duration_cast<chrono::nanoseconds>(TimeSinceEpoch).count();
	return 0.000000001 * DurationSinceEpoch; // to seconds
}
