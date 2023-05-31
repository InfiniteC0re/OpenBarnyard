#pragma once
#include "ModelHeader.h"

#include "Toshi/Utils/TLog.h"
#include "Toshi/Math/TVector2.h"
#include "Toshi/Math/TVector3.h"

#include <cstdint>

class Database
{
public:
	struct Vertex_t
	{
		Toshi::TVector3 Position;
		Toshi::TVector3 Normal;
		Toshi::TVector3 Color;
		Toshi::TVector2 UV;
	};

	struct MeshData_t
	{
		uint32_t m_Unknown;
		uint32_t m_NumFaces;
		uint32_t m_NumVertices1;
		uint32_t m_NumVertices2;
		char* m_MaterialName;
		Vertex_t* m_Vertices;
		uint16_t* m_Faces;
	};

	struct Mesh_t
	{
		Sphere_t m_Bounding;
		MeshData_t* m_Data;
	};

	struct RenderVolumeGroup_t
	{
		Sphere_t m_Bounding;
		RenderVolumeGroup_t* m_Next;
	};

	struct RenderVolumeGroupData_t
	{
		uint32_t m_NumMeshes;

		uint16_t GetMeshIndex(size_t index)
		{
			return *(reinterpret_cast<uint16_t*>(this + 1) + index);
		}
	};

	struct Model_t
	{
		char m_Data[132];
		uint32_t m_NumMeshes;
		Mesh_t** m_Meshes;
		RenderVolumeGroup_t* m_RenderGroups;
	};

	struct Entry_t
	{
		uint32_t m_ModelCount;
		Model_t** m_Models;
	};

public:
	Database()
	{
		m_NumEntries = 0;
		m_Entries = TNULL;
	}

	uint32_t GetNumEntries() const
	{
		return m_NumEntries;
	}

	Entry_t* operator[](size_t index)
	{
		TASSERT(index < m_NumEntries);
		return m_Entries[index];
	}

public:
	uint32_t m_NumEntries;
	Entry_t** m_Entries;
};