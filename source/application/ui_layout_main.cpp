#include "ui_layout_main.h"
#include "ui_core.h"

#include "ui_layout_ouliner.h"

void UI::DrawMainLayout(UIManager& ui)
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

		GLuint Texture, TextureWidth, TextureHeight;
		if (GLFramebuffers::GetTexture(ui.previewViewport->framebuffer, Texture, TextureWidth, TextureHeight))
		{
			ImGui::Image((ImTextureID)(intptr_t)Texture, ImVec2((float)TextureWidth, (float)TextureHeight), {0, 1}, {1, 0});
			ui.UpdateActiveViewport(ui.previewViewport, ImGui::IsItemHovered());
		}
	}
	ImGui::End();

	ImGui::SetNextWindowSize(ImVec2(App::settings.windowWidth * 1.0f, App::settings.windowHeight * 0.25f));
	ImGui::SetNextWindowPos(ImVec2(0, App::settings.windowHeight * 0.75f));
	ImGui::Begin("Nodes");
	{
		ImNodes::BeginNodeEditor();
			
		{
			ImNodes::BeginNode(1);

			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted("simple node :)");
			ImNodes::EndNodeTitleBar();

			ImNodes::BeginInputAttribute(2);
			ImGui::Text("input");
			ImNodes::EndInputAttribute();

			ImNodes::BeginOutputAttribute(3);
			ImGui::Indent(40);
			ImGui::Text("output");
			ImNodes::EndOutputAttribute();

			ImNodes::EndNode();
		}

		{
			ImNodes::BeginNode(4);

			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted("simple node 2");
			ImNodes::EndNodeTitleBar();

			ImNodes::BeginInputAttribute(5);
			ImGui::Text("input");
			ImNodes::EndInputAttribute();

			ImNodes::BeginOutputAttribute(6);
			ImGui::Indent(40);
			ImGui::Text("output");
			ImNodes::EndOutputAttribute();

			ImNodes::EndNode();
		}

		ImNodes::EndNodeEditor();
	}
	ImGui::End();

	ImGui::Begin("Outliner");
	UI::DisplayOutliner(App::world);
	ImGui::End();

	ImGui::Begin("Details");
	UI::DisplaySelectionDetails(App::ui.selected_object);
	ImGui::End();
}
