#include "application.h"
#include <atomic>
#include "opengl/framebuffer.h"

ApplicationSettings App::settings = ApplicationSettings();
ApplicationClock App::clock = ApplicationClock();
OpenGLWindow App::window = OpenGLWindow();
PythonInterpreter App::python = PythonInterpreter();

void App::Initialize()
{
	App::settings.windowRatio = App::settings.windowWidth / (float)App::settings.windowHeight;
	App::window.Initialize(App::settings.windowWidth, App::settings.windowHeight, App::settings.fullscreen, App::settings.vsync);
	App::python.Initialize();
	GLFramebuffers::Initialize(settings.windowWidth, settings.windowHeight, App::settings.clearColor);
}

void App::Shutdown()
{
	App::python.Shutdown();
	GLFramebuffers::Shutdown();
	App::window.Destroy();
	exit(0);
}

bool App::ReadyToTick()
{
	if (settings.fpsLimit <= 0)
		return true;

	double timeLimit = 1.0 / settings.fpsLimit;
	if (App::clock.TimeSinceLastTick() >= timeLimit)
		return true;

	if (App::settings.sleepWhenFpsLimited)
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

	return false;
}

void App::Tick()
{
	App::clock.Tick();

	ScriptExecutionResponse ScriptResponse;
	if (App::python.PopScriptResponse(ScriptResponse))
	{
		if (ScriptResponse.Error == PythonScriptError::None)
			std::cout << "Script executed fully" << std::endl;
		else
			std::cout << ScriptResponse.Exception.what();
	}
}
