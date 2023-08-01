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

void Viewport::HandleInputEvent(const void* eventPtr)
{
	SDL_Event& event = *((SDL_Event*)eventPtr);

	SDL_Keymod mod = SDL_GetModState();
	bool bCtrlModifier = mod & KMOD_CTRL;
	bool bShiftModifier = mod & KMOD_SHIFT;
	bool bAltModifier = mod & KMOD_ALT;

	if (event.type == SDL_KEYDOWN)
	{
		auto key = event.key.keysym.sym;

		if (key == SDLK_s) GLFramebuffers::SaveScreenshot(framebuffer);
		else if (key == SDLK_f) input.SnapToOrigin();

		else if (key == SDLK_KP_7) input.SetCameraView(CameraView::OrthographicY);
		else if (key == SDLK_KP_1) input.SetCameraView(bCtrlModifier ? CameraView::OrthographicZneg : CameraView::OrthographicZ);
		else if (key == SDLK_KP_9) input.SetCameraView(CameraView::OrthographicYneg);
		else if (key == SDLK_KP_3) input.SetCameraView(bCtrlModifier ? CameraView::OrthographicXneg : CameraView::OrthographicX);
		else if (key == SDLK_KP_5) input.SetCameraView(CameraView::Perspective);
	}

	if (event.type == SDL_MOUSEBUTTONDOWN)
	{
		App::ui.viewportCaptureMouse = true;
		App::ui.viewportCaptureMouseBeginX = event.motion.x;
		App::ui.viewportCaptureMouseBeginY = event.motion.y;
		SDL_ShowCursor(0);
		SDL_SetRelativeMouseMode(SDL_TRUE);

		auto button = event.button.button;
		if (button == SDL_BUTTON_LEFT)			input.inputState = TurntableInputState::Rotate;
		else if (button == SDL_BUTTON_MIDDLE)	input.inputState = TurntableInputState::Translate;
		else if (button == SDL_BUTTON_RIGHT)	input.inputState = TurntableInputState::Zoom;

		input.OnBeginInput();
	}
	else if (event.type == SDL_MOUSEBUTTONUP)
	{
		App::ui.viewportCaptureMouse = false;
		SDL_ShowCursor(1);
		SDL_SetRelativeMouseMode(SDL_FALSE);
	}
	else if (event.type == SDL_MOUSEMOTION && App::ui.viewportCaptureMouse)
	{
		input.ApplyMouseInput(-event.motion.xrel, event.motion.yrel, App::settings.viewportMouseSensitivity);
		SDL_WarpMouseInWindow(App::window.GetSDLWindow(), App::ui.viewportCaptureMouseBeginX, App::ui.viewportCaptureMouseBeginY);
	}
}
