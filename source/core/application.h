#pragma once
#include <filesystem>
#include "clock.h"

struct ApplicationSettings
{
	bool vsync = false;
	bool fullscreen = false;
	int windowWidth = 0;
	int windowHeight = 0;
	int fpsLimit = 0;
	float windowRatio = 0;
	std::filesystem::path contentPath;
};

class App
{
public:
	App() = delete;
	~App() = delete;

	static void Initialize();
	static void Tick();

	static ApplicationSettings settings;
	static ApplicationClock clock;
};