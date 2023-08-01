#include "viewport.h"
#include "application.h"

bool Viewport::hasInitialViewport = false;

void Viewport::Initialize()
{
	input.Initialize();

	// TODO: Remove once UI layout is good
	input.turntablePivot = glm::vec3{ -0.15f, 0.0f, 0.0f };
	input.Set(-65.0f, 15.0f, 0.75f);

	if (width == 0 || height == 0)
	{
		width = App::settings.windowWidth;
		height = App::settings.windowWidth;
	}

	if (hasInitialViewport)
	{
		framebuffer = GLFramebuffers::Create(width, height, App::settings.clearColor);
	}
	else
	{
		hasInitialViewport = true;
		framebuffer = 0;
	}
}

void Viewport::Destroy()
{
	input.Shutdown();

	if (framebuffer == 0)
	{
		hasInitialViewport = false;
	}
}

void Viewport::Resize(GLuint newWidth, GLuint newHeight)
{
	width = newWidth;
	height = newHeight;

	if (width <= 0)
		width = App::settings.windowWidth;

	if (height <= 0)
		width = App::settings.windowHeight;

	if (framebuffer)
	{
		GLFramebuffers::Resize(framebuffer, width, height);
	}
}
