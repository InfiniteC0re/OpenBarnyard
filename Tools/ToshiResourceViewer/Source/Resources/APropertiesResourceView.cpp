#include "pch.h"
#include "APropertiesResourceView.h"
#include "ImGui/imgui.h"

#include <Plugins/PPropertyParser/PPropertiesWriter.h>
#include <imgui/imgui_internal.h>

void APropertiesResourceView::ShowProperties(PProperties* a_pProperties)
{
	static constexpr int s_NodeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_FramePadding;

	TBOOL bUpdatedValue = TFALSE;
	auto fnUpdateValue = [&](PProperties::PProperty* a_pProperty, PPropertyValue* a_pValue)
	{
		bUpdatedValue = m_pSelectedProperty != a_pProperty || m_pSelectedValue != a_pValue;
		m_pSelectedProperty = a_pProperty;
		m_pSelectedValue = a_pValue;
	};

	for (auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++)
	{
		auto propName = it->GetName().GetString();
		const int iFlags = (m_pSelectedProperty == it) ? ImGuiTreeNodeFlags_Selected : 0;

		switch (it->GetValue()->GetType())
		{
		case PPropertyValue::Type::Int:
		case PPropertyValue::Type::String:
		case PPropertyValue::Type::Float:
		case PPropertyValue::Type::Bool:
		case PPropertyValue::Type::UInt32:
			ImGui::TreeNodeEx(it, s_NodeFlags | iFlags | ImGuiTreeNodeFlags_Leaf, propName);
			ImGui::TreePop();

			if (ImGui::IsItemFocused())
				fnUpdateValue(it, it->GetValue());

			break;
		case PPropertyValue::Type::Array:
			if (ImGui::TreeNodeEx(it, iFlags | s_NodeFlags, propName))
			{
				if (ImGui::IsItemFocused())
					fnUpdateValue(it, TNULL);

				auto pArray = it->GetValue()->GetArray();

				for (size_t i = 0; i < pArray->GetSize(); i++)
				{
					auto pItem = pArray->GetValue(i);

					const int iItemFlags = (m_pSelectedValue == pItem) ? ImGuiTreeNodeFlags_Selected : 0;

					switch (pItem->GetType())
					{
					case PPropertyValue::Type::Int:
						ImGui::TreeNodeEx(pItem, s_NodeFlags | iItemFlags | ImGuiTreeNodeFlags_Leaf, "%d", pItem->GetInteger());
						break;
					case PPropertyValue::Type::String:
						ImGui::TreeNodeEx(pItem, s_NodeFlags | iItemFlags | ImGuiTreeNodeFlags_Leaf, "%s", pItem->GetString());
						break;
					case PPropertyValue::Type::Float:
						ImGui::TreeNodeEx(pItem, s_NodeFlags | iItemFlags | ImGuiTreeNodeFlags_Leaf, "%f", pItem->GetFloat());
						break;
					case PPropertyValue::Type::Bool:
						ImGui::TreeNodeEx(pItem, s_NodeFlags | iItemFlags | ImGuiTreeNodeFlags_Leaf, "%s", pItem->GetBoolean() ? "true" : "false");
						break;
					case PPropertyValue::Type::UInt32:
						ImGui::TreeNodeEx(pItem, s_NodeFlags | iItemFlags | ImGuiTreeNodeFlags_Leaf, "%u", pItem->GetUINT32());
						break;
					}

					if (ImGui::IsItemFocused())
						fnUpdateValue(TNULL, pItem);

					ImGui::TreePop();
				}

				ImGui::TreePop();
			}
			else
			{
				if (ImGui::IsItemFocused())
					fnUpdateValue(it, TNULL);
			}

			break;
		case PPropertyValue::Type::Props:
			if (ImGui::TreeNodeEx(it, s_NodeFlags | iFlags, propName))
			{
				if (ImGui::IsItemFocused())
					fnUpdateValue(it, TNULL);

				ShowProperties(it->GetValue()->GetProperties());
				ImGui::TreePop();
			}
			else
			{
				if (ImGui::IsItemFocused())
					fnUpdateValue(it, TNULL);
			}


			break;
		}
	}

	if (bUpdatedValue)
	{
		if (m_pSelectedProperty)
		{
			Toshi::TStringManager::String8CopySafe(m_PropertyNameBuffer, m_pSelectedProperty->GetName().GetString());
		}

		if (m_pSelectedValue)
		{
			switch (m_pSelectedValue->GetType())
			{
			case PPropertyValue::Type::Int:
				Toshi::TStringManager::String8Format(m_PropertyValueBuffer, sizeof(m_PropertyValueBuffer), "%d", m_pSelectedValue->GetInteger());
				break;
			case PPropertyValue::Type::String:
				Toshi::TStringManager::String8Format(m_PropertyValueBuffer, sizeof(m_PropertyValueBuffer), "%s", m_pSelectedValue->GetString());
				break;
			case PPropertyValue::Type::Float:
				Toshi::TStringManager::String8Format(m_PropertyValueBuffer, sizeof(m_PropertyValueBuffer), "%f", m_pSelectedValue->GetFloat());
				break;
			case PPropertyValue::Type::Bool:
				Toshi::TStringManager::String8Format(m_PropertyValueBuffer, sizeof(m_PropertyValueBuffer), "%s", m_pSelectedValue->GetBoolean() ? "true" : "false");
				break;
			case PPropertyValue::Type::UInt32:
				Toshi::TStringManager::String8Format(m_PropertyValueBuffer, sizeof(m_PropertyValueBuffer), "%u", m_pSelectedValue->GetUINT32());
				break;
			}
		}
	}
}

void APropertiesResourceView::Render()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 0.0f));
	
	auto szWinName = Toshi::TStringManager::GetTempString8();
	Toshi::TStringManager::String8Format(szWinName, 128, "Properties Window - %s", GetFile()->GetFileName());
	ImGui::SetNextWindowSize({ 500, 300 }, ImGuiCond_FirstUseEver);
	ImGui::Begin(szWinName, TNULL, ImGuiWindowFlags_MenuBar);
	ImGui::PopStyleVar();

	ImGui::BeginMenuBar();
	if (ImGui::BeginMenu("File"))
	{
		ImGui::MenuItem("New");
		ImGui::MenuItem("Open...");

		static TBOOL s_bUseCompression = TFALSE;
		ImGui::Checkbox("Use BTEC compression", &s_bUseCompression);
		
		if (ImGui::MenuItem("Save"))
		{
			PPropertiesWriter::WriteTRB(GetFile()->GetFileName(), m_Properties, s_bUseCompression);
		}

		ImGui::MenuItem("Save as...");
		ImGui::Separator();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);
		
		if (ImGui::MenuItem("Close"))
		{
			Destroy();
		}

		ImGui::EndMenu();
	}
	ImGui::EndMenuBar();

	ImGuiID dockspaceId = ImGui::GetID("PPropertiesDockSpace");
	ImGui::DockSpace(dockspaceId);

	static TBOOL s_bInitialiseDockSpace = TTRUE;

	if (s_bInitialiseDockSpace)
	{
		ImGui::DockBuilderAddNode(dockspaceId,ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetContentRegionMaxAbs());

		ImGuiID dockIdEditor;
		ImGuiID dockIdList = ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Left, 0.6f, nullptr, &dockIdEditor);
		ImGui::DockBuilderGetNode(dockIdList)->LocalFlags = ImGuiDockNodeFlags_NoTabBar;
		ImGui::DockBuilderGetNode(dockIdEditor)->LocalFlags = ImGuiDockNodeFlags_NoTabBar;
		ImGui::DockBuilderDockWindow("PPropertyList", dockIdList);
		ImGui::DockBuilderDockWindow("PPropertyEditor", dockIdEditor);
		ImGui::DockBuilderFinish(dockspaceId);

		s_bInitialiseDockSpace = TFALSE;
	}

	if (ImGui::Button("Destroy"))
	{
		Destroy();
	}
	else
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4, 2 });
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 2 });
		ImGui::Begin("PPropertyList", TNULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
		if (ImGui::TreeNodeEx(&m_Properties, ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_CollapsingHeader, "Properties"))
		{
			ImGui::TreePush();
			ShowProperties(&m_Properties);
			ImGui::TreePop();
		}
		ImGui::PopStyleVar(2);
		ImGui::End();

		ImGui::Begin("PPropertyEditor", TNULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
		
		if (m_pSelectedProperty)
		{
			ImGui::Text("Property Name:");
			ImGui::InputText("##PropertyName", m_PropertyNameBuffer, sizeof(m_PropertyNameBuffer));
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4);
		}
		
		if (m_pSelectedValue)
		{
			switch (m_pSelectedValue->GetType())
			{
			case PPropertyValue::Type::Int:
				ImGui::Text("Property Value (Int):");
				ImGui::InputInt("##PropertyValue", m_pSelectedValue->GetIntegerPointer());
				break;
			case PPropertyValue::Type::String:
				ImGui::Text("Property Value (String):");
				ImGui::InputText("##PropertyValue", m_PropertyValueBuffer, sizeof(m_PropertyValueBuffer));
				ImGui::SameLine();
				
				if (ImGui::Button("Apply"))
				{
					m_pSelectedValue->SetString(m_PropertyValueBuffer);
				}

				break;
			case PPropertyValue::Type::Float:
				ImGui::Text("Property Value (Float):");
				ImGui::InputFloat("##PropertyValue", m_pSelectedValue->GetFloatPointer());
				break;
			case PPropertyValue::Type::Bool:
				ImGui::Text("Property Value (Boolean):");
				ImGui::SameLine();
				ImGui::Checkbox("##PropertyValue", m_pSelectedValue->GetBooleanPointer());
				break;
			case PPropertyValue::Type::UInt32:
				ImGui::Text("Property Value (UInt32):");
				ImGui::DragInt("##PropertyValue", TREINTERPRETCAST(int*, m_pSelectedValue->GetUINT32Pointer()));
				break;
			case PPropertyValue::Type::Array:
				ImGui::Text("Property Value (Array):");
				break;
			case PPropertyValue::Type::Props:
				ImGui::Text("Property Value (Properties):");
				break;
			}
		}
		
		ImGui::End();
	}

	ImGui::End();
}
