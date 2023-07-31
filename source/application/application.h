#pragma once
#include "core/core.h"
#include "opengl/opengl.h"
#include "python/python.h"

#include "shadermanager.h"
#include "geometrymanager.h"
#include "scripting.h"

#include "webcam.h"

#include "ui.h"

struct ApplicationSettings
{
	bool vsync = false;
	bool fullscreen = false;
	int windowWidth = 0;
	int windowHeight = 0;
	int maxFPS = 0;
	bool sleepWhenFpsLimited = true;
	float windowRatio = 0;
	glm::vec4 clearColor = glm::vec4(0.0f);
	float pointCloudSize = 0.001f;
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
	static void Render(WeakPtr<Camera> camera);

	static inline std::filesystem::path Path(const std::string& RelativePath) { return std::filesystem::current_path().parent_path() / std::filesystem::path(RelativePath); }

	static ApplicationSettings settings;
	static ApplicationClock clock;
	static OpenGLWindow window;
	static Scripting scripting;

	static UIManager ui;
	static ShaderManager shaders;
	static GeometryManager geometry;

	static UniformRandomGenerator random;

	static WeakPtr<Object> world;

	// TODO: move to draw debug manager
	static WeakPtr<GLLine> debuglines;
	static WebCam webcam;
};