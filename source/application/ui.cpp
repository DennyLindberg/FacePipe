#include "ui.h"
#include "ui_core.h"

#include "ui_layout_main.h"

void UIManager::Initialize()
{
	logging.Initialize();

	ImGuiIO& io = ImGui::GetIO();

	// fontawesome 6 example from https://github.com/juliettef/IconFontCppHeaders
	{
		io.Fonts->AddFontDefault();
		float baseFontSize = 16.0f; // 13.0f is the size of the default font. Change to the font size you use.
		float iconFontSize = baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

		// merge in icons from Font Awesome
		static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
		ImFontConfig icons_config;
		icons_config.MergeMode = true;
		icons_config.PixelSnapH = true;
		icons_config.GlyphMinAdvanceX = iconFontSize;

		std::filesystem::path fontPath(App::Path("content/fonts/fontawesome/Font Awesome 6 Free-Solid-900.otf"));
		io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), iconFontSize, &icons_config, icons_ranges);
	}

	applicationViewport = Viewport::Pool.CreateWeak();
	sceneViewport = Viewport::Pool.CreateWeak();
	sceneViewport->Resize(GLuint(App::settings.windowWidth*0.25f), GLuint(App::settings.windowHeight*0.25f));

	App::window.drawImguiCallback = [this]() -> void {
		UI::GenerateMainLayout(*this);
	};
}

void UIManager::Shutdown()
{
	applicationViewport.Destroy();

	for (WeakPtr<Viewport> viewport : viewports)
	{
		viewport.Destroy();
	}

	viewports.clear();

	logging.Shutdown();
}

bool UIManager::HandleInputEvent(const void* event)
{
	App::window.HandleImguiEvent((const SDL_Event*) event);

	if (Viewport* activeViewport = GetActiveViewport())
	{
		return false;
	}

	return true;
}

void UIManager::RenderUI()
{
	App::ui.applicationViewport->UseForRendering(EGLFramebufferClear::None);
	App::window.RenderImgui();
}

WeakPtr<Viewport> UIManager::CreateViewport()
{
	WeakPtr<Viewport> newViewport = Viewport::Pool.CreateWeak();
	viewports.push_back(newViewport);
	return newViewport;
}

void UIManager::UpdateActiveViewport(WeakPtr<Viewport> viewport, bool bActiveByMouse)
{
	if (bActiveByMouse)
		activeViewport = viewport;
	else
		activeViewport.Clear();
}

Viewport* UIManager::GetActiveViewport()
{
	if (Viewport* active = activeViewport)
	{
		return active;
	}
	else if (HasMouseFocus() || HasKeyboardFocus())
	{
		return nullptr;
	}

	return nullptr;

	// We don't use the application viewport for anything else than UI drawing
	//else
	//{
	//	return applicationViewport;
	//}
}

bool UIManager::HasKeyboardFocus() const
{
	return ImGui::GetIO().WantCaptureKeyboard;
}

bool UIManager::HasMouseFocus() const
{
	return ImGui::GetIO().WantCaptureMouse;
}

void UIManager::HandleQuit()
{
	if (displayQuitDialog)
	{
		displayQuitDialog = false;
	}
	else if (App::HasUnsavedChanges())
	{
		displayQuitDialog = true;
	}
	else
	{
		App::Quit();
	}
}
