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

	debuglines = GLLine::Pool.CreateWeak();
}

void Viewport::Destroy()
{
	input.Shutdown();

	debuglines.Destroy();

	if (framebuffer == 0)
	{
		hasInitialViewport = false;
	}
}

void Viewport::Clear(EGLFramebufferClear clear)
{
	GLFramebuffers::Bind(framebuffer);
	GLFramebuffers::ClearActive(clear);
}

void Viewport::UseForRendering(EGLFramebufferClear clear)
{
	GLFramebuffers::Bind(framebuffer);
	GLFramebuffers::ClearActive(clear);

	App::shaders.UpdateCameraUBO(input.camera);
	App::shaders.UpdateLightUBODirection(App::ui.lightFollowsCamera ? -input.camera->ForwardVector() : App::settings.skyLightDirection);
	App::shaders.UpdateLightUBOColor(App::settings.skyLightColor);

}

void Viewport::RenderScoped(EGLFramebufferClear clear, std::function<void()> func)
{
	if (auto F = GLFramebuffers::BindScoped(framebuffer))
	{
		App::ui.previewViewport->UseForRendering();

		GLFramebuffers::ClearActive(clear);

		if (func)
		{
			func();
		}
	}
}

void Viewport::Render(std::function<void(Viewport&)> func)
{
	UseForRendering(EGLFramebufferClear::All);

	// Background color gradient
	App::shaders.backgroundShader.Use();
	App::geometry.quad.Draw();
	Clear(EGLFramebufferClear::Depth);

	// Scene geometry
	glPolygonMode(GL_FRONT_AND_BACK, (App::ui.renderWireframe? GL_LINE : GL_FILL));
	func(*this);

	// Grid
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (input.camera->GetView() == CameraView::Perspective)
		App::geometry.grid.Draw(App::geometry.quad, input.camera->ViewProjectionMatrix());
	else
		App::geometry.grid.Draw(App::geometry.quad, input.camera->ViewProjectionMatrix(), input.camera->ForwardVector(), input.camera->SideVector());

	// Line overlays (coordinates, debug lines, etc)
	App::shaders.lineShader.Use();
	App::geometry.coordinateAxis.Draw();
	debuglines->SendToGPU();
	debuglines->Draw();
	debuglines->Clear();
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
