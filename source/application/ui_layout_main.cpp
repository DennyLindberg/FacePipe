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

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);

	ImGuiWindowFlags ChildFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

	float splitterThickness = 4.0f;

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Example: Fullscreen window", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		const float ws = ImGui::GetContentRegionAvail().x;
		const float hs = ImGui::GetContentRegionAvail().y;
		static float w = 0.33f;
		static float h = 0.8f;
		w = glm::clamp(w, 0.1f, 0.9f);
		h = glm::clamp(h, 0.1f, 0.9f);

		if (App::ui.fullscreenViewport)
		{
			ImGui::DrawViewport(&ui, ui.sceneViewport, 1.0f);
		}
		else
		{
			// Viewport region
			{
				ImGui::BeginChild("viewportregion", ImVec2(w * ws, h * hs), true);
					ImGui::DrawViewport(&ui, ui.sceneViewport, 1.0f);
				ImGui::EndChild();
			}

			// Region splitter next to viewport
			{
				ImGui::SameLine();
				ImGui::InvisibleButton("vsplitter", ImVec2(splitterThickness, h*hs));
				if (ImGui::IsItemHovered())
					ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
				if (ImGui::IsItemActive())
					w += ImGui::GetIO().MouseDelta.x/ws;
			}

			// Right region with middle/right divide
			{
				ImGui::SameLine();
				ImGui::BeginChild("child2", ImVec2(0, h*hs), true);
					const float ws2 = ImGui::GetContentRegionAvail().x;
					const float hs2 = ImGui::GetContentRegionAvail().y;
					static float w2 = 0.66f;
					w2 = glm::clamp(w2, 0.1f, 0.9f);

					// Center region
					{
						ImGui::BeginChild("centerregion", ImVec2(w2 * ws2, 0), true);
							UI::DisplayNodeGraph();
						ImGui::EndChild();
					}

					// Region splitter
					{
						ImGui::SameLine();
						ImGui::InvisibleButton("vsplitter2", ImVec2(splitterThickness, h * hs2));
						if (ImGui::IsItemHovered())
							ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
						if (ImGui::IsItemActive())
							w2 += ImGui::GetIO().MouseDelta.x / ws2;
					}

					// Right region
					{
						ImGui::SameLine();
						ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(3, 3));
						ImGui::BeginChild("detailsregion", ImVec2(0, 0), true);
							//UI::DisplayOutliner(App::world);
							//UI::DisplaySelectionDetails(App::ui.selected_object);

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
							ImGui::SliderFloat("PointSize", &App::settings.pointCloudSize, 0.0005f, 0.1f, "%.5f");
							ImGui::InputTextMultiline("ScriptInput", &input_field_string, ImVec2(0.0f, 100.0f));
							if (ImGui::Button("Execute"))
							{
								App::scripting.Execute(input_field_string);
								Logf(LOG_STDOUT, "Script is not implemented\n");
							}

							if (App::webcam.Texture())
							{
								float ratio = App::webcam.TextureWidth() / (float) App::webcam.TextureHeight();
								ImGui::Image((ImTextureID)(intptr_t)App::webcam.Texture(), ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().x/ratio), { 0, 1 }, { 1, 0 });
							}

							ImGui::Text("Mediapipe Time %.3f", App::latestFrame.Meta.Time);

							static bool bShowBlendshapes = true;
							ImGui::Checkbox("Show Blendshapes", &bShowBlendshapes);
							if (bShowBlendshapes)
							{
								ImGui::Text("Arkit Blendshapes");
								for (auto& Pair : App::latestFrame.Blendshapes)
								{
									ImGui::SliderFloat(Pair.first.c_str(), &Pair.second, -1.0f, 1.0f, "%.3f", ImGuiSliderFlags_NoInput);
								}
							}
							else
							{
								ImGui::Text("Mediapipe Landmarks");
								for (size_t i = 0; i < App::latestFrame.Landmarks.size(); ++i)
								{
									ImGui::SliderFloat(("##" + std::to_string(i)).c_str(), &App::latestFrame.Landmarks[i], -1.0f, 1.0f, "%.3f", ImGuiSliderFlags_NoInput);
								}
							}


						ImGui::EndChild();
						ImGui::PopStyleVar();
						ImGui::PopStyleVar();
					}
				ImGui::EndChild();
			}

			// Region splitter above log
			{
				ImGui::InvisibleButton("hsplitter", ImVec2(-1, splitterThickness));
				if (ImGui::IsItemHovered())
					ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
				if (ImGui::IsItemActive())
					h += ImGui::GetIO().MouseDelta.y/hs;
			}

			// Log region
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(3, 3));
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
					ImGui::BeginChild("logregion", ImVec2(0, 0), true);
						ui.logging.Draw(NULL, false);
					ImGui::EndChild();
				ImGui::PopStyleVar();
				ImGui::PopStyleVar();
			}
		}
	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void UI::GenerateMainMenuBar(UIManager& ui)
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ImGui::MenuItem("New", "CTRL+N", false, false);
			ImGui::MenuItem("Open", "CTRL+O", false, false);
			ImGui::MenuItem("Save", "CTRL+S", false, false);
			ImGui::MenuItem("Save As", "CTRL+SHIFT+S", false, false);
			if (ImGui::MenuItem("Quit", ""))
			{
				ui.HandleQuit();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z", false, false))
			{
				std::cout << "Undo" << std::endl;
			}
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X", false, false)) {}
			if (ImGui::MenuItem("Copy", "CTRL+C", false, false)) {}
			if (ImGui::MenuItem("Paste", "CTRL+V", false, false)) {}
			ImGui::EndMenu();
		}

		ImGui::TextUnformatted("(?)");
		if (ImGui::BeginItemTooltip())
		{
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted("Control camera    [Mouse buttons]");
			ImGui::TextUnformatted("Toggle Fullscreen [F]");
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
