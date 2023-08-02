#include "application.h"
#include <atomic>
#include "opengl/framebuffer.h"

bool App::bQuit = false;
bool App::bUnsavedChanges = false;

ApplicationSettings App::settings = ApplicationSettings();
ApplicationClock App::clock = ApplicationClock();
OpenGLWindow App::window = OpenGLWindow();
Scripting App::scripting = Scripting();

UIManager App::ui = UIManager();
ShaderManager App::shaders = ShaderManager();
GeometryManager App::geometry = GeometryManager();

UniformRandomGenerator App::random = UniformRandomGenerator();

WeakPtr<Object> App::world = WeakPtr<Object>();
WebCam App::webcam = WebCam();

namespace ObjectPoolInternals
{
	void InitializeDefaultPools()
	{
		Register<Camera, ObjectType_Camera>();
		//Register<Light, ObjectType_Light>();
		Register<GLTexture, ObjectType_GLTexture>();
		Register<GLQuad, ObjectType_GLQuad>();
		Register<GLLine, ObjectType_GLLine>();
		Register<GLLineStrips, ObjectType_GLLineStrips>();
		Register<GLTriangleMesh, ObjectType_GLTriangleMesh>();
		Register<GLBezierStrips, ObjectType_GLBezierStrips>();
	}
}

void App::Initialize()
{
	Logging::StartLoggingThread();
	
	App::settings.windowRatio = App::settings.windowWidth / (float)App::settings.windowHeight;
	App::window.Initialize(App::settings.windowWidth, App::settings.windowHeight, App::settings.fullscreen, App::settings.vsync);
	App::scripting.Initialize();

	ObjectPoolInternals::InitializeDefaultPools();

	GLFramebuffers::Initialize(settings.windowWidth, settings.windowHeight, App::settings.clearColor);
	App::shaders.Initialize(App::Path("content/shaders"));
	App::geometry.Initialize();
	App::ui.Initialize();

	App::world = Object::Pool.CreateWeak();
	App::world->name = "World";

	App::webcam.Initialize();
}

void App::Shutdown()
{
	App::webcam.Shutdown();

	ObjectPoolInternals::ShutdownPools();

	App::ui.Shutdown();
	App::shaders.Shutdown();
	App::geometry.Shutdown();
	GLFramebuffers::Shutdown();

	App::scripting.Shutdown();
	App::window.Destroy();

	Logging::StopLoggingThread();

	exit(0);
}

bool App::ReadyToTick()
{
	if (settings.maxFPS <= 0)
		return true;

	double timeLimit = 1.0 / settings.maxFPS;
	if (App::clock.TimeSinceLastTick() >= timeLimit)
		return true;

	if (App::settings.sleepWhenFpsLimited)
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

	return false;
}

void App::Tick()
{
	App::clock.Tick();
	App::webcam.UpdateTextureWhenDirty();
	App::scripting.Tick();
	App::shaders.Tick();

	Logging::Flush();

	std::string logLine;
	while (Logging::GetLine(logLine))
	{
		App::ui.logger.AddLog(logLine.c_str());
	}
}

void App::Quit()
{
	bQuit = true;
}
