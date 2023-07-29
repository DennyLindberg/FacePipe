#include "application.h"
#include <atomic>

ApplicationSettings App::settings = ApplicationSettings();
ApplicationClock App::clock = ApplicationClock();

void App::Initialize()
{

}

void App::Tick()
{
	App::clock.Tick();
}
