#include "pch.h"

// TOSHI_TMEMORY_FLAGS allows you to change the mode TMemory initializes with
// Flags_NativeMethods stands for native memory management methods instead of using dlmalloc
#define TOSHI_TMEMORY_FLAGS Toshi::TMemory::Flags_Standard

// TOSHI_TMEMORY_SIZE allows you to change size of the global memory block
// The value is not used when TMemory initialized with native methods
#define TOSHI_TMEMORY_SIZE 64 * 1024 * 1024

// This file includes the entrypoint so set all the settings before including it
#include "Toshi.h"

#include <Toshi/File/TTRB.h>
#include <Toshi/Render/TModelManager.h>
#include <Toshi/Render/TTexture.h>
#include <Toshi/Strings/TPString8.h>

#include <Plugins/PTRB.h>
#include <Plugins/PPropertyParser/PProperties.h>

#include "ModelHeader.h"
#include "Materials.h"
#include "Database.h"

#include <limits>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE(AnimControllerType);
TPSTRING8_DECLARE(AnimObjTypes);

int TMain(int argc, char** argv)
{
	TTRB trb;
	
	if (TTRB::ERROR_OK == trb.Load("C:\\Stuff\\Barnyard\\Game\\Data\\AGopherShopGame.trb"))
	{
		auto properties = PProperties::LoadFromTRB(trb);
		auto disablebreakpoints = properties->GetOptionalProperty("playercharacter")->GetProperties()->GetOptionalProperty("disablebreakpoints")->GetBoolean();
	}

	{
		PProperties properties;
		auto stringArray = properties.AddPropertyArray("string_array");
		stringArray->GetValue()->GetArray()->Add("string1");
		stringArray->GetValue()->GetArray()->Add("string2");
		stringArray->GetValue()->GetArray()->Add("string3");

		auto subprop = properties.AddProperties("unit");
		subprop->GetValue()->GetProperties()->AddProperty("TClass", "ASteer");

		TOSHI_INFO("string_array[0] = {}", properties.GetOptionalProperty("string_array")->GetArray()->GetValue(0)->GetString());
		TOSHI_INFO("string_array[1] = {}", properties.GetOptionalProperty("string_array")->GetArray()->GetValue(1)->GetString());
		TOSHI_INFO("string_array[2] = {}", properties.GetOptionalProperty("string_array")->GetArray()->GetValue(2)->GetString());
		TOSHI_INFO("unit.TClass = {}", properties.GetOptionalProperty("unit")->GetProperties()->GetOptionalProperty("TClass")->GetString());
	}

	return 0;
	/*TSystemManager::CreateStringPool();
	TPString8Pool::Create();

	{
		TPString8 newString("TPString8 test");
		TOSHI_INFO("Dynamic pooled string: {}", newString.GetString8());
		TOSHI_INFO("{}", newString.GetString8() == TPString8("TPString8 test").GetString8());
	}

	const TPString8* testString1 = TPS8(AnimControllerType);
	const TPString8* testString2 = TPS8(AnimObjTypes);

	TOSHI_INFO("Static pooled strings: {}, {}", testString1->GetString8(), testString2->GetString8());

	return 0;*/

	PTRB::TRBF network;
	network.ReadFromFile("C:\\Stuff\\Barnyard\\Game\\Data\\Networks\\park2.trb");

	struct INetwork
	{
		struct S1
		{
			enum Type
			{
				Type_Locator        = 0,
				Type_Trigger        = 4,
				Type_TerrainTrigger = 8  // ?
			};

			TVector3 vec1;
			TVector3 vec2;
			uint32_t unk1; // m_pInteriorNetwork
			uint32_t index;
			Type type;
			struct {
				TVector3 v1;
				TVector3 v2;
			}* unk4;
			uint32_t unk5;
			char* unk6;
			union {
				struct {
					char* unk7;
					char* locatorName;
				};

				struct {
					TVector2 vec3;
				};
			};
			char* triggerName;
			uint32_t unk8;
		};

		struct S2
		{
			char padding[32];
			void* m_pInteriorNetwork;
			TVector3 m_oPosition;
			int m_iIndex;
			const char* m_Name1;
			const char* m_Name2;
			int unk;
		};

		uint32_t num1;
		uint32_t numItems;
		uint32_t num2;
		uint32_t num3;
		S1* items;
		S2* items2;
		char* names1;
		char* names2;
		char* names3;
		uint32_t unk4;
		uint32_t unk5;
		uint32_t unk6;
	};

	auto pSect = network.GetSECT();
	auto pSymb = network.GetSYMB();

	auto inetwork = pSymb->Find<INetwork>(pSect, "INetwork").get();

	//sizeof(INetwork::S2);

	for (size_t i = 0; i < inetwork->numItems; i++)
	{
		auto& item = inetwork->items2[i];
		TOSHI_INFO("{} {} - {} {} {}", item.m_Name1, item.m_Name2, item.m_oPosition.x, item.m_oPosition.y, item.m_oPosition.z);
	}

	//for (size_t i = 0; i < inetwork->numItems; i++)
	//{
	//	auto& item = inetwork->items[i];

	//	//if (item.type != INetwork::S1::Type_Trigger && item.type != INetwork::S1::Type_Locator)
	//	//	__debugbreak();

	//	if (item.type & INetwork::S1::Type_Trigger)
	//	{
	//		if (strcmp(item.triggerName, "park2.triggers.sleep") == 0)
	//			__debugbreak();

	//		TOSHI_INFO(
	//			"Trigger: {} - ({} {} {}) x ({} {} {}) - {}",
	//			item.triggerName,
	//			item.vec1.x, item.vec1.y, item.vec1.z,
	//			item.vec2.x, item.vec2.y, item.vec2.z,
	//			item.type
	//		);
	//	}

	//	if (item.type == INetwork::S1::Type_Locator)
	//	{
	//		TOSHI_INFO(
	//			"Locator: {} - ({} {} {}) x ({} {} {}) - {}",
	//			item.locatorName,
	//			item.vec1.x, item.vec1.y, item.vec1.z,
	//			item.vec2.x, item.vec2.y, item.vec2.z,
	//			item.type
	//		);
	//	}

	//	if (item.type & INetwork::S1::Type_TerrainTrigger)
	//	{
	//		TOSHI_INFO(
	//			"Terrain Trigger: {} - ({} {} {}) x ({} {} {}) - {}",
	//			item.triggerName,
	//			item.vec1.x, item.vec1.y, item.vec1.z,
	//			item.vec2.x, item.vec2.y, item.vec2.z,
	//			item.type
	//		);
	//	}
	//}
	
	/*std::vector<std::string> strings;
	char* cursor = inetwork->names1;

	for (size_t i = 0; i < inetwork->num1 + inetwork->numLocators + inetwork->num2 + inetwork->num3; i++)
	{
		std::string currentString;

		while (*cursor != '\0')
			currentString += *(cursor++);

		cursor++;
		strings.push_back(currentString);
	}*/

	/*TLib::TRBF::TRBF fsms;
	TLib::TRBF::TRBF fsmsD;
	fsms.ReadFromFile("C:\\Stuff\\Barnyard\\Game\\Data\\Units\\allfsms.trb");
	fsmsD.ReadFromFile("C:\\Stuff\\Barnyard\\Game\\Data\\Units\\allfsmsD.trb");

	struct FSM
	{
		struct Main
		{
			unsigned m_Count1;
			unsigned m_uiNumKeywords;
			uint8_t* m_InstructionsData;
			const char** m_Strings;
			unsigned m_uiInstructionsDataSize;
			const char** m_Keywords;
		};
	};

	auto lineNums = fsmsD.GetSYMB()->Find<uint32_t>(fsmsD.GetSECT(), "LineNums").get();

	auto pSect = fsms.GetSECT();
	auto pSymb = fsms.GetSYMB();

	auto main = pSymb->Find<FSM::Main>(pSect, "Main").get();

	auto s1 = main->m_Strings[0];*/

	return 0;

	MaterialList materialList;
	uint32_t meshCount = 0;

	constexpr bool bSkipGrassModels = false;

	std::vector<PTRB::TRBF*> trbfiles = {
		new PTRB::TRBF("C:\\Stuff\\Barnyard\\Game\\Data\\Terrain\\EnvMainO\\Barn_L0Mod0.trb"),
		new PTRB::TRBF("C:\\Stuff\\Barnyard\\Game\\Data\\Terrain\\EnvMainO\\Barn_L0Mod1.trb"),
		new PTRB::TRBF("C:\\Stuff\\Barnyard\\Game\\Data\\Terrain\\EnvMainO\\Barn_L0Mod2.trb"),
		new PTRB::TRBF("C:\\Stuff\\Barnyard\\Game\\Data\\Terrain\\EnvMainO\\Barn_L0Mod3.trb"),
		new PTRB::TRBF("C:\\Stuff\\Barnyard\\Game\\Data\\Terrain\\EnvMainO\\BarnNorth_L0Mod0.trb"),
		new PTRB::TRBF("C:\\Stuff\\Barnyard\\Game\\Data\\Terrain\\EnvMainO\\BarnNorth_L0Mod1.trb"),
		new PTRB::TRBF("C:\\Stuff\\Barnyard\\Game\\Data\\Terrain\\EnvMainO\\BarnNorth_L0Mod2.trb"),
		new PTRB::TRBF("C:\\Stuff\\Barnyard\\Game\\Data\\Terrain\\EnvMainO\\BarnNorth_L0Mod3.trb"),
		new PTRB::TRBF("C:\\Stuff\\Barnyard\\Game\\Data\\Terrain\\EnvMainO\\BarnNorth_L0Mod4.trb"),
		new PTRB::TRBF("C:\\Stuff\\Barnyard\\Game\\Data\\Terrain\\EnvMainO\\Hilltop_L0Mod0.trb"),
		new PTRB::TRBF("C:\\Stuff\\Barnyard\\Game\\Data\\Terrain\\EnvMainO\\Hilltop_L0Mod1.trb"),
	};

	for (auto trbf : trbfiles)
	{
		auto header = trbf->GetSYMB()->Find<ModelHeader>(trbf->GetSECT(), "Header");
		auto materials = trbf->GetSYMB()->Find<Materials>(trbf->GetSECT(), "Materials");

		if (bSkipGrassModels && header->m_Shader == ModelHeader::MODELSHADER_GRASS)
		{
			TOSHI_INFO("Skipping grass model...");
			continue;
		}

		meshCount += header->m_MeshCount;

		for (size_t i = 0; i < materials->m_Count; i++)
		{
			Materials::Material_t* material = ((Materials::Material_t*)(materials.get() + 1)) + i;
			materialList.AddMaterial(material->MaterialName, material->MaterialFile);
		}
	}

	PTRB::TRBF outTrbf;
	PTRB::SECT* sect = outTrbf.GetSECT();
	PTRB::SYMB* symb = outTrbf.GetSYMB();

	auto stack = sect->CreateStack();

	auto header = stack->Alloc<ModelHeader>();

	header->m_LodDistance = 100.0f;
	header->m_MeshCount = meshCount;
	header->m_ModelCount = 1;
	header->m_Shader = ModelHeader::MODELSHADER_WORLD;
	header->m_Unknown = 0;

	auto database = stack->Alloc<Database>();
	database->m_NumEntries = 1;

	stack->Alloc(&database->m_Entries, database->m_NumEntries);

	auto materials = stack->Alloc<Materials>();
	*materials.get() = materialList.GetData();

	for (size_t i = 0; i < materialList.GetMaterialCount(); i++)
	{
		auto material = stack->Alloc<Materials::Material_t>();
		memcpy(material.get(), materialList[i], sizeof(Materials::Material_t));
	}

	auto entry = stack->Alloc(&database->m_Entries[0]);

	entry->m_ModelCount = 1;
	stack->Alloc(&entry->m_Models, entry->m_ModelCount);
	auto model = stack->Alloc(&entry->m_Models[0]);

	auto renderGroup = stack->Alloc(&model->m_RenderGroups);
	renderGroup->m_Next = TNULL;

	auto renderGroupData = stack->Alloc<Database::RenderVolumeGroupData_t>();
	renderGroupData->m_NumMeshes = meshCount;

	auto meshIndices = stack->Alloc<uint16_t>(renderGroupData->m_NumMeshes);
	for (uint16_t m = 0; m < renderGroupData->m_NumMeshes; m++)
		meshIndices.get()[m] = m;

	model->m_NumMeshes = renderGroupData->m_NumMeshes;
	memset(model->m_Data, 0, sizeof(model->m_Data));

	stack->Alloc(&model->m_Meshes, model->m_NumMeshes);

	float minX, minY, minZ;
	float maxX, maxY, maxZ;
	minX = minY = minZ = std::numeric_limits<float>::infinity();
	maxX = maxY = maxZ = -std::numeric_limits<float>::infinity();

	size_t meshIndex = 0;
	for (auto trbf : trbfiles)
	{
		auto originalHeader = trbf->GetSYMB()->Find<ModelHeader>(trbf->GetSECT(), "Header");
		auto originalDB = trbf->GetSYMB()->Find<Database>(trbf->GetSECT(), "Database");

		if (bSkipGrassModels && originalHeader->m_Shader == ModelHeader::MODELSHADER_GRASS)
			continue;

		for (size_t i = 0; i < originalDB->m_NumEntries; i++)
		{
			auto originalEntry = originalDB->m_Entries[i];

			for (size_t k = 0; k < originalEntry->m_ModelCount; k++)
			{
				auto originalModel = originalEntry->m_Models[k];

				for (size_t j = 0; j < originalModel->m_NumMeshes; j++)
				{
					auto originalMesh = originalModel->m_Meshes[j];
					auto mesh = stack->Alloc(&model->m_Meshes[meshIndex++]);

					mesh->m_Bounding = originalMesh->m_Bounding;
					auto data = stack->Alloc(&mesh->m_Data);

					auto originalData = originalMesh->m_Data;
					data->m_NumFaces = originalData->m_NumFaces;
					data->m_NumVertices1 = originalData->m_NumVertices1;
					data->m_NumVertices2 = originalData->m_NumVertices2;
					data->m_Unknown = originalData->m_Unknown;

					stack->Alloc(&data->m_MaterialName, strlen(originalData->m_MaterialName) + 1);
					strcpy(data->m_MaterialName, originalData->m_MaterialName);

					stack->Alloc(&data->m_Vertices, data->m_NumVertices1);
					memcpy(data->m_Vertices, originalData->m_Vertices, sizeof(*data->m_Vertices) * data->m_NumVertices1);

					stack->Alloc(&data->m_Faces, data->m_NumFaces);
					memcpy(data->m_Faces, originalData->m_Faces, sizeof(*data->m_Faces) * data->m_NumFaces);

					// Calculate render volume sphere
					auto numVertices = originalMesh->m_Data->m_NumVertices1;

					for (size_t v = 0; v < numVertices; v++)
					{
						auto& vertex = originalMesh->m_Data->m_Vertices[v];
						minX = std::min(minX, vertex.Position.x);
						minY = std::min(minY, vertex.Position.y);
						minZ = std::min(minZ, vertex.Position.z);
						maxX = std::max(maxX, vertex.Position.x);
						maxY = std::max(maxY, vertex.Position.y);
						maxZ = std::max(maxZ, vertex.Position.z);
					}

					TVector3 sphereCenter = { minX + (maxX - minX) / 2, minY + (maxY - minY) / 2, minZ + (maxZ - minZ) / 2 };
					float radius = std::max(std::max((maxX - minX) / 2, (maxY - minY) / 2), (maxZ - minZ) / 2);

					header->m_RenderVolume = renderGroup->m_Bounding = { sphereCenter, radius };
				}
			}
		}
	}

	symb->Add(stack, "Database", database.get());
	symb->Add(stack, "Header", header.get());
	symb->Add(stack, "Materials", materials.get());
	symb->Add(stack, "Collision", stack->Alloc<int>().get());

	outTrbf.WriteToFile("C:\\Stuff\\Barnyard\\Game\\Data\\Terrain\\EnvMain\\Hilltop_L0MrgW.trb");

	//if (trbf.ReadFromFile("C:\\Stuff\\Barnyard\\Game\\Data\\Terrain\\EnvMain\\barnnorth_L0Mod0.trb"))
	//{
	//	auto originalHeader = trbf.GetSYMB()->Find<ModelHeader>(trbf.GetSECT(), "Header");
	//	auto originalDB = trbf.GetSYMB()->Find<Database>(trbf.GetSECT(), "Database");

	//	TLib::TRBF::TRBF outTrbf;
	//	TLib::TRBF::SECT* sect = outTrbf.GetSECT();
	//	TLib::TRBF::SYMB* symb = outTrbf.GetSYMB();

	//	auto stack = sect->CreateStack();

	//	auto header = stack->Alloc<ModelHeader>();

	//	header->m_LodDistance  = originalHeader->m_LodDistance;
	//	header->m_MeshCount    = originalHeader->m_MeshCount;
	//	header->m_ModelCount   = originalHeader->m_ModelCount;
	//	header->m_Shader       = ModelHeader::MODELSHADER_WORLD;
	//	header->m_Unknown      = originalHeader->m_Unknown;

	//	auto database = stack->Alloc<Database>();
	//	database->m_NumEntries = originalDB->m_NumEntries;

	//	stack->Alloc(&database->m_Entries, database->m_NumEntries);

	//	auto originalMaterials = trbf.GetSYMB()->Find<Materials>(trbf.GetSECT(), "Materials");
	//	auto materials = stack->Alloc<Materials>();
	//	materials->m_Zero1 = 0;
	//	materials->m_Zero2 = 0;
	//	materials->m_Count = originalMaterials->m_Count;
	//	materials->m_Size = originalMaterials->m_Size;

	//	for (size_t i = 0; i < originalMaterials->m_Count; i++)
	//	{
	//		auto material = stack->Alloc<Materials::Material_t>();
	//		memcpy(material.get(), originalMaterials->GetMaterial(i), sizeof(Materials::Material_t));
	//	}

	//	for (size_t i = 0; i < database->m_NumEntries; i++)
	//	{
	//		auto originalEntry = originalDB->m_Entries[i];
	//		auto entry = stack->Alloc(&database->m_Entries[i]);

	//		entry->m_ModelCount = originalEntry->m_ModelCount;
	//		stack->Alloc(&entry->m_Models, entry->m_ModelCount);

	//		for (size_t k = 0; k < entry->m_ModelCount; k++)
	//		{
	//			auto originalModel = originalEntry->m_Models[k];
	//			auto model = stack->Alloc(&entry->m_Models[k]);

	//			auto renderGroup = stack->Alloc(&model->m_RenderGroups);

	//			auto renderGroupData = stack->Alloc<Database::RenderVolumeGroupData_t>();
	//			renderGroupData->m_NumMeshes = originalModel->m_NumMeshes;

	//			auto meshIndices = stack->Alloc<uint16_t>(renderGroupData->m_NumMeshes);
	//			for (uint16_t m = 0; m < renderGroupData->m_NumMeshes; m++)
	//				meshIndices.get()[m] = m;

	//			model->m_NumMeshes = originalModel->m_NumMeshes;
	//			memcpy(model->m_Data, originalModel->m_Data, sizeof(model->m_Data));

	//			stack->Alloc(&model->m_Meshes, model->m_NumMeshes);

	//			for (size_t j = 0; j < model->m_NumMeshes; j++)
	//			{
	//				auto originalMesh = originalModel->m_Meshes[j];
	//				auto mesh = stack->Alloc(&model->m_Meshes[j]);

	//				mesh->m_Bounding = originalMesh->m_Bounding;
	//				auto data = stack->Alloc(&mesh->m_Data);

	//				auto originalData = originalMesh->m_Data;
	//				data->m_NumFaces = originalData->m_NumFaces;
	//				data->m_NumVertices1 = originalData->m_NumVertices1;
	//				data->m_NumVertices2 = originalData->m_NumVertices2;
	//				data->m_Unknown = originalData->m_Unknown;

	//				stack->Alloc(&data->m_MaterialName, strlen(originalData->m_MaterialName) + 1);
	//				strcpy(data->m_MaterialName, originalData->m_MaterialName);

	//				stack->Alloc(&data->m_Vertices, data->m_NumVertices1);
	//				memcpy(data->m_Vertices, originalData->m_Vertices, sizeof(*data->m_Vertices) * data->m_NumVertices1);

	//				stack->Alloc(&data->m_Faces, data->m_NumFaces);
	//				memcpy(data->m_Faces, originalData->m_Faces, sizeof(*data->m_Faces) * data->m_NumFaces);
	//			}

	//			{
	//				// Calculate render volume sphere
	//				float minX, minY, minZ;
	//				float maxX, maxY, maxZ;

	//				minX = minY = minZ = std::numeric_limits<float>::infinity();
	//				maxX = maxY = maxZ = -std::numeric_limits<float>::infinity();

	//				for (size_t j = 0; j < originalModel->m_NumMeshes; j++)
	//				{
	//					auto mesh = originalModel->m_Meshes[j];
	//					auto numVertices = mesh->m_Data->m_NumVertices1;

	//					for (size_t v = 0; v < numVertices; v++)
	//					{
	//						auto& vertex = mesh->m_Data->m_Vertices[v];
	//						minX = std::min(minX, vertex.Position.x);
	//						minY = std::min(minY, vertex.Position.y);
	//						minZ = std::min(minZ, vertex.Position.z);
	//						maxX = std::max(maxX, vertex.Position.x);
	//						maxY = std::max(maxY, vertex.Position.y);
	//						maxZ = std::max(maxZ, vertex.Position.z);
	//					}
	//				}

	//				TVector3 sphereCenter = { minX + (maxX - minX) / 2, minY + (maxY - minY) / 2, minZ + (maxZ - minZ) / 2 };
	//				float radius = std::max(
	//					std::max((maxX - minX) / 2, (maxY - minY) / 2),
	//					(maxZ - minZ) / 2
	//				);
	//				
	//				header->m_RenderVolume = renderGroup->m_Bounding = { sphereCenter, radius };
	//				renderGroup->m_Next = TNULL;
	//			}
	//		}
	//	}
	//}

	return 0;
}