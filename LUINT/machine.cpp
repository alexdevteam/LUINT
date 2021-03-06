#include "machine.h"
#include "lua.hpp"
#include <imgui.h>
#include "gui.h"
#include <algorithm>
#include "hostdata.h"
#include <iostream>

using namespace LUINT;

namespace LUINT::Machines
{
	Machine::Machine(Data::SessionData& _session, std::string _name, Network* _network) : session(&_session), name(_name), uid(UID::generate()), network(_network)
	{
		network->add_machine(this);
	}

	Machine::~Machine()
	{
		// Remove the machine from any connections.

		if (network == nullptr)
			return;

		network->remove_machine(this);
	}

#pragma region Rendering

	void Machine::ShowMachineInfo()
	{
		{
			char machineInfoName[64];
			sprintf_s(machineInfoName, 64, "Machine Information###%s", uid.as_string().c_str());
			if (!ImGui::Begin(machineInfoName, &showMachineInfo))
			{
				ImGui::End();
				return;
			}
		}

		if (editingName)
		{
			static char buf[32] = "";
			if (ImGui::InputTextWithHint("", "Input new machine name...", buf, 32, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				name = buf;
				editingName = false;
			}
		}
		else
		{
			ImGui::Text("Machine name: %s", name.c_str());
			ImGui::SameLine();
			editingName = ImGui::Button("Change");
		}

		ImGui::Text("Machine manufacturer: %s", get_info().manufacturer);

		ImGui::TextUnformatted("Description:");
		ImGui::TextWrapped(get_info().description);
		if (ImGui::CollapsingHeader("Advanced information"))
		{
			ImGui::Text("UID: %s", uid.as_string().c_str());
			ImGui::Text("Lua version: %s", LUA_VERSION);
			if(network)
				ImGui::Text("Connected Network UID: %s", network->get_uid().as_string().c_str());
			else
				ImGui::TextUnformatted("Connected Network UID: <nil>");
		}
		ImGui::End();
	}
	
	void Machine::Render()
	{
		ImGuiIO& io = ImGui::GetIO();

		if (showMachineInfo)
			ShowMachineInfo();

		RenderChildWindows();

		char buf[MAX_MACHINENAME_LENGTH + 32];
		sprintf_s(buf, MAX_MACHINENAME_LENGTH + 32, "%s###m%s", GetWindowName().c_str(), uid.as_string().c_str()); // Use ### to have an unique identifier (even when the machine changes its name)

		bool p_open = true;
		if (!ImGui::Begin(buf, &p_open, GetWindowFlags()))
		{
			// Early out if the window is collapsed, as an optimization.
			ImGui::End();
			return;
		}

		if (!p_open)
			v_shouldDelete = true;

		windowPos = ImGui::GetWindowPos();
		windowSize = ImGui::GetWindowSize();

		if (ImGui::BeginMenuBar())
		{
			RenderMenuItems();
			AddDefaultMenuItems();
			ImGui::EndMenuBar();
		}

		RenderWindow();

		char cannotConnectToItselfID[16];
		sprintf_s(cannotConnectToItselfID, 16, "n%s", uid.as_string("%x").c_str());

		if (session->connecting != nullptr)
		{
			if (ImGui::IsWindowHovered())
			{
				ImVec2 min = ImGui::GetWindowPos();
				ImVec2 max = ImVec2(min.x + ImGui::GetWindowSize().x, min.y + ImGui::GetWindowSize().y);
				ImGui::GetForegroundDrawList()->AddRect(min, max, ImGui::GetColorU32(ImGuiCol_PlotHistogramHovered), 3, 15, 5.0f);

				if (ImGui::IsMouseClicked(0))
				{
					if (session->connecting == network)
					{
						ImGui::OpenPopup(cannotConnectToItselfID);
					}
					else
					{
						// Switch network to new one
						network->remove_machine(this);
						session->connecting->add_machine(this);

						OnChangeNetwork(network, session->connecting);

						network = session->connecting;
						session->connecting->add_machine(this);
						session->connecting = nullptr;
					}
				}

				if (ImGui::IsMouseReleased(0))
					showCannotConnectToItselfTooltip = false;
			}
		}

		if (ImGui::BeginPopup(cannotConnectToItselfID))
		{
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted("Cannot connect a machine to itself.");
			ImGui::PopTextWrapPos();
			ImGui::EndPopup();
		}

		ImGui::End();
	}

	void Machine::RenderWindow()
	{
		ImGui::TextWrapped("This is a default render text -- Override Machine::RenderWindow to have control of what is shown here!");
	}

	void Machine::AddDefaultMenuItems()
	{
		if (ImGui::BeginMenu("Connections"))
		{
			if (session->connecting == network)
			{
				if (ImGui::MenuItem("Connect", "Ctrl+LMB", true))
				{
					session->connecting = nullptr;
				}
			}
			else if (session->connecting == nullptr)
			{
				if (ImGui::MenuItem("Connect", "Ctrl+LMB", false))
				{
					session->connecting = network;
				}
			}
			else
			{
				ImGui::MenuItem("Connect", "Ctrl+LMB", false, false);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("About"))
		{
			ImGui::MenuItem("Machine Information", "Ctrl+I", &showMachineInfo);
			ImGui::EndMenu();
		}
	}

#pragma endregion Rendering
}
