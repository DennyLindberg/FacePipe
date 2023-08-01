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
}
