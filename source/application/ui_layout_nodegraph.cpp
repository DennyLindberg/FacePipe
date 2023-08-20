#include "ui_layout_nodegraph.h"
#include "ui_core.h"

#include "imnodes.h"

namespace UI
{
	void DisplayNodeGraph()
	{
		static std::string SpinnerTemplate = "            ";

		ImNodes::BeginNodeEditor();

		static bool bInitialLoad = true;
		if (bInitialLoad)
		{
			bInitialLoad = false;
			ImNodes::SetNodeGridSpacePos(1, ImVec2(25.0f, 50.0f));
			ImNodes::SetNodeGridSpacePos(2, ImVec2(250.0f, 225.0f));
			ImNodes::SetNodeGridSpacePos(3, ImVec2(400.0f, 400.0f));
		}

		{
			static int ReceiveCounter = 0;
			static int SpinnerPos = 0;
			if (App::receiveDataSocket.bReceivedDataLastCall)
			{
				ReceiveCounter++;

				int LastIndex = (int) (SpinnerTemplate.size() - 1);

				if (ReceiveCounter <= LastIndex)
				{
					SpinnerPos = ReceiveCounter;
				}
				else
				{
					SpinnerPos = LastIndex - (ReceiveCounter%SpinnerTemplate.size()); // flip direction

					if (SpinnerPos <= 0)
					{
						ReceiveCounter = 0;
						SpinnerPos = 0;
					}
				}
			}

			ImNodes::BeginNode(1);
				ImNodes::BeginNodeTitleBar();
					ImGui::PushStyleColor(ImGuiCol_Text, App::receiveDataSocket.IsConnected()? IM_COL32(0, 255, 0, 255) : IM_COL32(0, 0, 0, 255));
						ImGui::TextUnformatted(ICON_FA_WIFI);
					ImGui::PopStyleColor();
					ImGui::SameLine(0, 5);
					ImGui::TextUnformatted(App::receiveDataSocket.ToString().c_str());
				ImNodes::EndNodeTitleBar();

				ImNodes::BeginOutputAttribute(1);
					// connection info
					{
						FacePipe::MessageInfo& meta = App::lastReceivedDatagram.metaData;
						ImGui::Text("%s", meta.Source.c_str());
						ImGui::Text("Scene: %d", meta.Scene);
						ImGui::Text("Camera: %d", meta.Camera);
						ImGui::Text("Subject: %d", meta.Subject);
						ImGui::Text("Time: %.2f", meta.Time);
					}

					// spinner
					{
						//std::string spinnermodified = SpinnerTemplate;
						//spinnermodified[SpinnerPos] = '*';
						//ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(100, 100, 100, 255));
						//	ImGui::Text("%s", spinnermodified.c_str());
						//ImGui::PopStyleColor();
						//ImGui::SameLine(0, 5);
						//ImGui::Text("Receive");
					}
				ImNodes::EndOutputAttribute();
			ImNodes::EndNode();

		}

		{
			ImNodes::BeginNode(2);
				ImNodes::BeginNodeTitleBar();
					ImGui::TextUnformatted("example");
				ImNodes::EndNodeTitleBar();

				ImNodes::BeginInputAttribute(2);
				ImNodes::EndInputAttribute();

				ImNodes::BeginOutputAttribute(3);
				ImNodes::EndOutputAttribute();
			ImNodes::EndNode();
		}

		{
			ImNodes::BeginNode(3);
				ImNodes::BeginNodeTitleBar();
					ImGui::PushStyleColor(ImGuiCol_Text, false? IM_COL32(0, 255, 0, 255) : IM_COL32(0, 0, 0, 255));
						ImGui::TextUnformatted(ICON_FA_WIFI);
					ImGui::PopStyleColor();
					ImGui::SameLine(0, 5);
					ImGui::TextUnformatted("?.?.?.?:????");
				ImNodes::EndNodeTitleBar();

				ImNodes::BeginInputAttribute(4);
					ImGui::Text("Send");
					ImGui::SameLine(0, 5);
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(100, 100, 100, 255));
						ImGui::Text("%s", SpinnerTemplate.c_str());
					ImGui::PopStyleColor();
				ImNodes::EndInputAttribute();
			ImNodes::EndNode();
		}

		ImNodes::Link(1, 1, 2);
		ImNodes::Link(2, 3, 4);

		ImNodes::EndNodeEditor();
	}
}
