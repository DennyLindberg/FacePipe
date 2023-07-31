#pragma once
#include "core/core.h"
#include "opengl/opengl.h"
#include "python/python.h"

#include "shadermanager.h"
#include "geometrymanager.h"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imnodes.h"

// see objectptr.h
#define ObjectType_Object 1
#define ObjectType_Camera 2
#define ObjectType_Light 3
#define ObjectType_Mesh 4
#define ObjectType_Texture 5
#define ObjectType_Line 6

namespace Type
{
}

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
	static PythonInterpreter python;

	static ShaderManager shaders;
	static GeometryManager geometry;

	static UniformRandomGenerator random;

	static WeakPtr<Object> world;

	// TODO: move to draw debug manager
	static WeakPtr<GLLine> debuglines;
};