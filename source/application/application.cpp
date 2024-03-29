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
UDPSocket App::receiveDataSocket = UDPSocket();
UDPDatagram App::lastReceivedDatagram = UDPDatagram();
ThreadSafeQueue<UDPDatagram> App::datagramsQueue = ThreadSafeQueue<UDPDatagram>();

FacePipe::Frame App::latestFrame = FacePipe::Frame();

std::function<void(float, float, const SDL_Event& event)> App::OnTickEvent = [](float time, float dt, const SDL_Event& event) -> void {};
std::function<void(float, float)> App::OnTickScene = [](float time, float dt) -> void {};
std::function<void(float, float)> App::OnTickRender = [](float time, float dt) -> void {};

std::thread receiveDatagramsThread;
std::atomic<bool> shutdownReceiveThread = false;
void ReceiveDatagramsThreadLoop()
{
	while (!shutdownReceiveThread)
	{
		std::vector<UDPDatagram> grams;
		App::receiveDataSocket.Receive(grams);

		for (UDPDatagram& d : grams)
		{
			App::datagramsQueue.Push(d);
		}

		Sleep(1);
	}
}

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
	App::window.Initialize(App::settings.windowWidth, App::settings.windowHeight, App::settings.fullscreen, App::settings.vsync, App::settings.showConsole);
	App::window.SetTitle(App::settings.windowTitle);
	OpenGLWindow::OnWindowChanged = [](EGLWindowEvent event, int low, int high) -> void {
		if (event == EGLWindowEvent::Resize)
		{
			App::settings.windowWidth = low;
			App::settings.windowHeight = high;
			GLFramebuffers::Resize(0, low, high, true);
		}
		else if (event == EGLWindowEvent::SizeMoveTimer)
		{
			App::Tick();
		}
	};
	App::scripting.Initialize();

	ObjectPoolInternals::InitializeDefaultPools();

	GLFramebuffers::Initialize(settings.windowWidth, settings.windowHeight, App::settings.clearColor);
	App::shaders.Initialize(App::Path("content/shaders"));
	App::geometry.Initialize();
	App::ui.Initialize();

	App::ui.logging.Register(LOG_NET, "Network");
	App::ui.logging.Register(LOG_NET_SEND, "Net Send");
	App::ui.logging.Register(LOG_NET_RECEIVE, "Net Receive");
	UDPSocket::Logger = [](const char* str) -> void { App::ui.logging.AddLog(LOG_NET, str); };

	if (Net::StartWinsock() != 0)
	{
		Logf(LOG_STDOUT, "Failed to initialize Winsock\n");
	}

	App::world = Object::Pool.CreateWeak();
	App::world->name = "World";

	App::webcam.Initialize();

	receiveDataSocket.Set(Net::LocalHost, App::settings.receiveDataSocketPort);
	receiveDataSocket.Start();

	receiveDatagramsThread = std::thread(ReceiveDatagramsThreadLoop);
}

void App::Shutdown()
{
	shutdownReceiveThread = true;
	if (receiveDatagramsThread.joinable())
		receiveDatagramsThread.join();

	receiveDataSocket.Close();

	App::webcam.Shutdown();

	ObjectPoolInternals::ShutdownPools();

	Net::StopWinsock();

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

void App::Run()
{
	while (App::Tick()) {}
	App::Shutdown();
}

bool App::Tick()
{
	if (App::ReadyToQuit())
		return false;

	if (!App::ReadyToTick())
		return true;

	// Framebuffer size changes are deferred to this point in the tick to not mess with ongoing rendering
	GLFramebuffers::UpdateDirtyTextures();
	
	// Internals
	App::clock.Tick();
	App::webcam.UpdateTextureWhenDirty();
	App::scripting.Tick();
	App::shaders.Tick();
	App::ui.logging.Tick();
	
	// Input
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
			App::ui.HandleQuit();

		if (App::ui.HandleInputEvent(&event))
			continue;

		SDL_Keymod mod = SDL_GetModState();
		bool bCtrlModifier = mod & KMOD_CTRL;
		bool bShiftModifier = mod & KMOD_SHIFT;
		bool bAltModifier = mod & KMOD_ALT;

		if (event.type == SDL_KEYDOWN)
		{
			auto key = event.key.keysym.sym;

			// global keys here
			if (key == SDLK_f || key == SDLK_F11)
			{
				App::ui.fullscreenViewport = !App::ui.fullscreenViewport;
			}
		}

		if (Viewport* activeViewport = App::ui.GetActiveViewport())
		{
			activeViewport->HandleInputEvent((const void*)&event);
		}

		App::OnTickEvent((float) App::clock.time, (float) App::clock.deltaTime, event);
	}

	// Scene
	App::OnTickScene((float) App::clock.time, (float) App::clock.deltaTime);

	// Rendering
	App::ui.applicationViewport->Clear();
	App::OnTickRender((float) App::clock.time, (float) App::clock.deltaTime);
	App::ui.RenderUI();

	// End frame
	App::window.SwapFramebuffer();

	return true;
}

void App::Quit()
{
	bQuit = true;
}
