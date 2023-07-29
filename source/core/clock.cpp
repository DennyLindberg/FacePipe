#include "clock.h"
#include "SDL2/SDL.h"

ApplicationClock::ApplicationClock()
{
	time = SystemTime();
	Tick();
}

void ApplicationClock::Tick()
{
	sdl_ms_previous = sdl_ms_current;
	sdl_ms_current = SDL_GetPerformanceCounter();
	uint64_t sdl_ms_delta = sdl_ms_current - sdl_ms_previous;

	deltaTime = (double)(sdl_ms_delta / (double)SDL_GetPerformanceFrequency());
	lastTickTime = time + 0.0; // atomic bullshit
	time = SystemTime();
}

double ApplicationClock::SystemTime()
{
	return SDL_GetTicks64() / 1000.0;
}
