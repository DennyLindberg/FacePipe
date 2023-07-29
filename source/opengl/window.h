#pragma once

#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"

#include <string>
#include <functional>

class OpenGLWindow
{
protected:
	SDL_GLContext maincontext = nullptr;
	SDL_Window* window = nullptr;

public:
	OpenGLWindow() {}
	~OpenGLWindow() {}

	void SetTitle(std::string newCaption);
	void SwapFramebuffer();

	void HandleImguiEvent(const SDL_Event* event);

	void RenderImgui();

	std::function<void()> imguiLayout = [](auto&&...) {};

	void Initialize(int width, int height, bool fullscreenEnabled, bool vsync);
	void Destroy();
};
