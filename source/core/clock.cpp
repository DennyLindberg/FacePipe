#include "clock.h"

namespace chrono = std::chrono;

ApplicationClock::ApplicationClock()
{
	applicationStartTime = SecondsSinceEpoch();
	time = TimeSinceAppStart();
	deltaTime = 0.1; // non-zero init
}

void ApplicationClock::Tick()
{
	double previousTime = time;
	time = TimeSinceAppStart();
	deltaTime = time - previousTime;
}

double ApplicationClock::SecondsSinceEpoch()
{
	auto TimeSinceEpoch = chrono::system_clock::now().time_since_epoch();
	auto DurationSinceEpoch = chrono::duration_cast<chrono::nanoseconds>(TimeSinceEpoch).count();
	return 0.000000001 * DurationSinceEpoch; // to seconds
}
