#include "application/application.h"

namespace fs = std::filesystem;

/*
	Application
*/
int main(int argc, char* args[])
{
	App::settings = {
		.showConsole = false,
		.fullscreen = false,
		.windowWidth = 1280,
		.windowHeight = 720,
		.maxFPS = 60
	};
	auto& settings = App::settings;

	App::Initialize();
	App::window.SetTitle("ImguiDemo");

	App::window.drawImguiCallback = []() {
		ImGui::ShowDemoWindow();
	};

	while (!App::ReadyToQuit())
	{
		if (!App::ReadyToTick())
		{
			continue;
		}

		App::Tick();

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				App::Quit();

			if (App::ui.HandleInputEvent(&event))
				continue;
		}

		App::ui.RenderUI();

		App::window.SwapFramebuffer();
	}

	App::Shutdown();

	return 0;
}
