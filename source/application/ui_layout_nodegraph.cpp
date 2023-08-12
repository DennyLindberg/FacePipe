#include "ui_layout_nodegraph.h"
#include "ui_core.h"

#include "imnodes.h"

namespace UI
{
	void DisplayNodeGraph()
	{
		ImNodes::BeginNodeEditor();

		{
			ImNodes::BeginNode(1);

			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted("UDP");
			ImNodes::EndNodeTitleBar();

			ImNodes::EndNode();

			ImNodes::SetNodeGridSpacePos(1, ImVec2(0.0f, 0.0f));
		}

		{
			ImNodes::BeginNode(2);

			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted("example");
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
			ImNodes::BeginNode(3);
			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted("example");
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

		static bool bInitialLoad = true;
		if (bInitialLoad)
		{
			bInitialLoad = false;
			ImNodes::SetNodeGridSpacePos(3, ImVec2(300.0f, 300.0f));
		}

		ImNodes::Link(1, 3, 5);

		ImNodes::EndNodeEditor();
	}
}
