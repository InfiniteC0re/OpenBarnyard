#pragma once

#include "Toshi/File/TFile.h"
#include "Toshi/Render/TSkeleton.h"

namespace Toshi
{

	class TModelLOD
	{
		int m_iMeshCount; // 0x10
	public:
		inline int GetMeshCount() { return m_iMeshCount; }
	};

	class TModel
	{
		unsigned int m_Flags; // 0x4
		//int m_instanceCount; // 0x8
		int m_iLODCount; // 0xC
		TSkeleton* m_pSkeleton; // 0x14
		//int* m_pUVHashOffset; // 0x18
		TModelLOD* m_modelLODs; // 0x18
		void* m_pTRB; // 0xA4

		struct Header
		{
			uint32_t m_magic;
			uint32_t m_version;
			uint32_t m_remainingBytes;
		};

		struct Header2
		{

		};

		inline bool IsCreated() { return (m_Flags & 1) != 0; }

		inline TModelLOD& GetLOD(int index) { return m_modelLODs[index]; }
		inline int GetLODCount() { return m_iLODCount; }

	protected:
		bool LoadTMD(const char*); // JPOG only i think not sure though
		bool LoadTRBTMD(const char*);
	};

	
}


