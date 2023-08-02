#include "ui_layout_main.h"
#include "ui_core.h"

#include "ui_layout_ouliner.h"
#include "ui_layout_nodegraph.h"

#include "imgui_internal.h"

void UI::GenerateMainLayout(UIManager& ui)
{
	UI::GenerateMainMenuBar(ui);
	UI::GenerateStatusBar(ui);
	UI::GenerateSaveDialog(ui);

	static std::string input_field_string = "";

	// Fill everything

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	//ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x*0.25f, viewport->WorkSize.y));

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Example: Fullscreen window", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
	//ImGui::Begin("##LeftColumn", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
	ImGui::BeginChild("##LeftColumn", ImVec2(viewport->WorkSize.x*0.25f, viewport->WorkSize.y), true, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
	{
		ImGui::PopStyleVar();

		ImGui::DrawViewport(&ui, ui.sceneViewport, 0.5f);

		if (App::webcam.IsActive())
		{
			if (ImGui::Button("Stop"))
				App::webcam.Stop();
			ImGui::SameLine();
			ImGui::Text(App::webcam.DebugString().c_str());
		}
		else
		{
			if (ImGui::Button("Start Camera"))
				App::webcam.Start();
		}
				
		ImGui::InputInt("MaxFPS", &App::settings.maxFPS, 0);
		ImGui::Checkbox("Wireframe", &ui.renderWireframe);
		ImGui::Checkbox("Light follows camera", &ui.lightFollowsCamera);
		ImGui::SliderFloat("PointSize", &App::settings.pointCloudSize, 0.0005f, 0.01f, "%.5f");
		ImGui::InputTextMultiline( "ScriptInput", &input_field_string, ImVec2(0.0f, 100.0f) );
		if (ImGui::Button("Execute"))
		{
			App::scripting.Execute(input_field_string);
			Logf(LOG_STDOUT, "Script is not implemented\n");
		}

		if (App::webcam.Texture())
		{
			ImGui::Image((ImTextureID)(intptr_t)App::webcam.Texture(), ImVec2(App::webcam.TextureWidth() * 0.25f, App::webcam.TextureHeight() * 0.25f), { 0, 1 }, { 1, 0 });
		}

		ImGui::EndChild();
	}
	ImGui::End();

	//ImGui::SetNextWindowSize(ImVec2(App::settings.windowWidth * 1.0f, App::settings.windowHeight * 0.25f));
	//ImGui::SetNextWindowPos(ImVec2(0, App::settings.windowHeight * 0.75f));

	//ImGui::Begin("Nodes");
	//UI::DisplayNodeGraph();
	//ImGui::End();

	ImGui::Begin("Outliner");
	UI::DisplayOutliner(App::world);
	ImGui::End();

	ImGui::Begin("Details");
	UI::DisplaySelectionDetails(App::ui.selected_object);
	ImGui::End();

	ui.logging.Draw();
}

void UI::GenerateMainMenuBar(UIManager& ui)
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ImGui::MenuItem("New", "CTRL+N");
			ImGui::MenuItem("Open", "CTRL+O");
			ImGui::MenuItem("Save", "CTRL+S");
			ImGui::MenuItem("Save As", "CTRL+SHIFT+S");
			if (ImGui::MenuItem("Quit", ""))
			{
				ui.HandleQuit();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z"))
			{
				std::cout << "Undo" << std::endl;
			}
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			ImGui::EndMenu();
		}

		ImGui::TextUnformatted("(?)");
		if (ImGui::BeginItemTooltip())
		{
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted("Control camera    [Mouse buttons]");
			ImGui::TextUnformatted("Re-center camera  [F]");
			ImGui::TextUnformatted("Take Screenshot   [S]");
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}

		ImGui::EndMainMenuBar();
	}
}

void UI::GenerateStatusBar(UIManager& ui)
{
	// Thanks to https://github.com/ocornut/imgui/issues/3518

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_MenuBar;

	float height = ImGui::GetFrameHeight();
	if (ImGui::BeginViewportSideBar("##StatusBar", NULL, ImGuiDir_Down, height, window_flags)) 
	{
		if (ImGui::BeginMenuBar()) 
		{
			// Text right align thanks to https://stackoverflow.com/a/66109051
			std::string text = "FPS: " + FpsString(App::clock.deltaTime);
			auto posX = (ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(text.c_str()).x
				- ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
			if (posX > ImGui::GetCursorPosX())
				ImGui::SetCursorPosX(posX);
			ImGui::Text(text.c_str());

			ImGui::EndMenuBar();
		}
	}
	ImGui::End();
}

void UI::GenerateSaveDialog(UIManager& ui)
{
	static const char* save_popup = "SaveChanges?";
	if (ui.displayQuitDialog && !ImGui::IsPopupOpen(save_popup))
	{
		ImGui::OpenPopup(save_popup);
	}

	static std::vector<const char*> save_choices = { "Save", "Don't Save", "Cancel" };
	ImGui::OnPopupModalSave(save_popup, ICON_FA_FLOPPY_DISK" (?)", "Save changes before closing?", save_choices, [](const char* button) {
		if (button == save_choices[0])
		{
			App::SaveChanges();
			std::cout << "SAVED!";
		}

		if (button == save_choices[2])
		{
			App::ui.displayQuitDialog = false;
			ImGui::CloseCurrentPopup();
		}
		else
		{
			App::Quit();
		}
	});
}
