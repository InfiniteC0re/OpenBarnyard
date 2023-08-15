#include "pch.h"
#include "APropertiesResourceView.h"
#include "ImGui/imgui.h"

void ShowProperties(const PProperties* properties)
{
	for (auto it = properties->Begin(); it != properties->End(); it++)
	{
		auto propName = it->GetName().GetString();

		switch (it->GetValue()->GetType())
		{
		case PPropertyValue::Type::Int:
		case PPropertyValue::Type::String:
		case PPropertyValue::Type::Float:
		case PPropertyValue::Type::Bool:
		case PPropertyValue::Type::UInt32:
			ImGui::TreeNodeEx(it, ImGuiTreeNodeFlags_Leaf, propName);
			ImGui::TreePop();

			break;
		case PPropertyValue::Type::Array:
			if (ImGui::TreeNodeEx(propName))
			{
				auto pArray = it->GetValue()->GetArray();

				for (size_t i = 0; i < pArray->GetSize(); i++)
				{
					auto pItem = pArray->GetValue(i);

					switch (pItem->GetType())
					{
					case PPropertyValue::Type::Int:
						ImGui::TreeNodeEx(pItem, ImGuiTreeNodeFlags_Leaf, "%d", pItem->GetInteger());
						break;
					case PPropertyValue::Type::String:
						ImGui::TreeNodeEx(pItem, ImGuiTreeNodeFlags_Leaf, "%s", pItem->GetString());
						break;
					case PPropertyValue::Type::Float:
						ImGui::TreeNodeEx(pItem, ImGuiTreeNodeFlags_Leaf, "%f", pItem->GetFloat());
						break;
					case PPropertyValue::Type::Bool:
						ImGui::TreeNodeEx(pItem, ImGuiTreeNodeFlags_Leaf, "%s", pItem->GetBoolean() ? "true" : "false");
						break;
					case PPropertyValue::Type::UInt32:
						ImGui::TreeNodeEx(pItem, ImGuiTreeNodeFlags_Leaf, "%u", pItem->GetUINT32());
						break;
					}

					ImGui::TreePop();
				}

				ImGui::TreePop();
			}

			break;
		case PPropertyValue::Type::Props:
			if (ImGui::TreeNodeEx(it, 0, propName))
			{
				ShowProperties(it->GetValue()->GetProperties());
				ImGui::TreePop();
			}

			break;
		}
	}
}

void APropertiesResourceView::Render()
{
	ImGui::Begin("Properties Window");

	if (ImGui::Button("Destroy"))
	{
		Destroy();
	}
	else
	{
		if (ImGui::TreeNodeEx(&m_Properties, ImGuiTreeNodeFlags_CollapsingHeader, "Properties"))
		{
			ImGui::TreePush();
			ShowProperties(&m_Properties);
			ImGui::TreePop();
		}
	}


	ImGui::End();
}
