#pragma once
#include "core/core.h"
#include "opengl/opengl.h"
#include "python/python.h"
#include "imgui.h"
#include "imgui_stdlib.h"
#include "imnodes.h"

struct ApplicationSettings
{
	bool vsync = false;
	bool fullscreen = false;
	int windowWidth = 0;
	int windowHeight = 0;
	int fpsLimit = 0;
	bool sleepWhenFpsLimited = true;
	float windowRatio = 0;
	std::filesystem::path contentPath;
	glm::vec4 clearColor = glm::vec4(0.0f);
};

class App
{
public:
	App() = delete;
	~App() = delete;

	static void Initialize();
	static void Shutdown();
	static bool ReadyToTick();
	static void Tick();

	static ApplicationSettings settings;
	static ApplicationClock clock;
	static OpenGLWindow window;
	static PythonInterpreter python;
};