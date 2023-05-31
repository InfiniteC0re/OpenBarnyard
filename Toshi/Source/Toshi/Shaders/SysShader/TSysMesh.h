#pragma once
#include "Toshi/Core/TObject.h"
#include "Toshi/Render/TMesh.h"

namespace Toshi {
	
	class TSysMesh : 
		public TGenericClassDerived<TSysMesh, TMesh, "TSysMesh", TMAKEVERSION(1, 0), TFALSE>
	{
	public:
		TSysMesh();

		virtual TBOOL Create(uint32_t unk1, uint16_t uiMaxVertices, uint16_t uiMaxIndices);

		TBOOL Allocate();

	private:
		uint32_t m_Unk1;
		uint16_t m_uiMaxVertices;
		uint16_t m_uiMaxIndices;
		void* m_pVertexPool;
		void* m_pIndexPool;
	};

}