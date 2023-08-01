#include "ui.h"
#include "ui_core.h"

#include "ui_layout_main.h"

void UIManager::Initialize()
{
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

	previewFramebuffer = GLFramebuffers::Create(GLuint(App::settings.windowWidth*0.25f), GLuint(App::settings.windowHeight*0.25f), App::settings.clearColor);

	App::window.drawImguiCallback = [this]() -> void {
		DrawUI();
	};
}

void UIManager::Shutdown()
{

}

bool UIManager::HandleInputEvent(const void* event)
{
	App::window.HandleImguiEvent((const SDL_Event*) event);
	if (HasKeyboardFocus() && !App::ui.interactingWithPreview)
	{
		return true;
	}

	return false;
}

void UIManager::DrawUI()
{
	UI::DrawMainLayout(*this);
}

bool UIManager::HasKeyboardFocus() const
{
	return ImGui::GetIO().WantCaptureKeyboard;
}

bool UIManager::HasMouseFocus() const
{
	return ImGui::GetIO().WantCaptureMouse;
}
