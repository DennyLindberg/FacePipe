#include "ui_layout_main.h"
#include "ui_core.h"

#include "ui_layout_ouliner.h"
#include "ui_layout_nodegraph.h"

#include "imgui_internal.h"

void UI::GenerateMainLayout(UIManager& ui)
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
		ImGui::EndMainMenuBar();
	}

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::Begin("Example: Fullscreen window", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

		ui.logger.Draw("Log");

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

	ImGui::End();
}


void UI::GenerateMainLayout_Deprecated(UIManager& ui)
{
	std::string helpString = R"(Controls:
		- Mouse buttons: Camera
		- S: Screenshot
		- F: Re-center camera
	)";

	static std::string input_field_string = "";

	ImGui::SetNextWindowSize(ImVec2(App::settings.windowWidth * 0.25f, App::settings.windowHeight * 1.0f));
	ImGui::SetNextWindowPos(ImVec2(0, 0));

	ImGui::Begin("App");
	{
		ImGui::Text("Scene");
		ImGui::Text(("App - FPS: " + FpsString(App::clock.deltaTime)).c_str());
		ImGui::Text(("App - Time: " + std::to_string(App::clock.time)).c_str());
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
		}

		ImGui::InputTextMultiline( "Help", &helpString, ImVec2(0.0f, 100.0f), ImGuiInputTextFlags_ReadOnly );

		if (App::webcam.Texture())
		{
			ImGui::Image((ImTextureID)(intptr_t)App::webcam.Texture(), ImVec2(App::webcam.TextureWidth() * 0.25f, App::webcam.TextureHeight() * 0.25f), { 0, 1 }, { 1, 0 });
		}

		ImGui::DrawViewport(&ui, ui.previewViewport);
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

	ui.logger.Draw("Log");
}
