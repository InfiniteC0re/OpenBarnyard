#include "pch.h"

// TOSHI_TMEMORY_FLAGS allows you to change the mode TMemory initializes with
// Flags_NativeMethods stands for native memory management methods instead of using dlmalloc
#define TOSHI_TMEMORY_FLAGS Toshi::TMemory::Flags_NativeMethods

// TOSHI_TMEMORY_SIZE allows you to change size of the global memory block
// The value is not used when TMemory initialized with native methods
#define TOSHI_TMEMORY_SIZE 0

// This file includes the entrypoint so set all the settings before including it
#include "Toshi.h"

#include "Toshi/File/TTRB.h"
#include "Toshi/Render/TModelManager.h"
#include "Toshi/Render/TTexture.h"
#include "TRBF/TRBF.h"

#include "ModelHeader.h"
#include "Materials.h"
#include "Database.h"

#include <limits>

using namespace Toshi;

int TMain(int argc, char** argv)
{
	TLib::TRBF::TRBF fsms;
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

	auto s1 = main->m_Strings[0];

	return 0;

	MaterialList materialList;
	uint32_t meshCount = 0;

	constexpr bool bSkipGrassModels = false;

	std::vector<TLib::TRBF::TRBF*> trbfiles = {
		new TLib::TRBF::TRBF("C:\\Stuff\\Barnyard\\Game\\Data\\Terrain\\EnvMainO\\Barn_L0Mod0.trb"),
		new TLib::TRBF::TRBF("C:\\Stuff\\Barnyard\\Game\\Data\\Terrain\\EnvMainO\\Barn_L0Mod1.trb"),
		new TLib::TRBF::TRBF("C:\\Stuff\\Barnyard\\Game\\Data\\Terrain\\EnvMainO\\Barn_L0Mod2.trb"),
		new TLib::TRBF::TRBF("C:\\Stuff\\Barnyard\\Game\\Data\\Terrain\\EnvMainO\\Barn_L0Mod3.trb"),
		new TLib::TRBF::TRBF("C:\\Stuff\\Barnyard\\Game\\Data\\Terrain\\EnvMainO\\BarnNorth_L0Mod0.trb"),
		new TLib::TRBF::TRBF("C:\\Stuff\\Barnyard\\Game\\Data\\Terrain\\EnvMainO\\BarnNorth_L0Mod1.trb"),
		new TLib::TRBF::TRBF("C:\\Stuff\\Barnyard\\Game\\Data\\Terrain\\EnvMainO\\BarnNorth_L0Mod2.trb"),
		new TLib::TRBF::TRBF("C:\\Stuff\\Barnyard\\Game\\Data\\Terrain\\EnvMainO\\BarnNorth_L0Mod3.trb"),
		new TLib::TRBF::TRBF("C:\\Stuff\\Barnyard\\Game\\Data\\Terrain\\EnvMainO\\BarnNorth_L0Mod4.trb"),
		new TLib::TRBF::TRBF("C:\\Stuff\\Barnyard\\Game\\Data\\Terrain\\EnvMainO\\Hilltop_L0Mod0.trb"),
		new TLib::TRBF::TRBF("C:\\Stuff\\Barnyard\\Game\\Data\\Terrain\\EnvMainO\\Hilltop_L0Mod1.trb"),
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

	TLib::TRBF::TRBF outTrbf;
	TLib::TRBF::SECT* sect = outTrbf.GetSECT();
	TLib::TRBF::SYMB* symb = outTrbf.GetSYMB();

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