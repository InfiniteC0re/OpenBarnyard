#include "ToshiPCH.h"
#include "TSysMesh.h"
#include "Toshi/Render/TRender.h"

namespace Toshi {

	TSysMesh::TSysMesh()
	{
		m_Unk1 = 0;
		m_uiMaxVertices = 0;
		m_uiMaxIndices = 0;
		m_pVertexPool = TNULL;
		m_pIndexPool = TNULL;
	}

	TBOOL TSysMesh::Create(uint32_t unk1, uint16_t uiMaxVertices, uint16_t uiMaxIndices)
	{
		TASSERT(TFALSE == IsCreated());

		m_Unk1 = unk1;
		m_uiMaxVertices = uiMaxVertices;
		m_uiMaxIndices = uiMaxIndices;

		if (Allocate())
		{

			return TTRUE;
		}
		else
		{
			return TFALSE;
		}
	}

	TBOOL TSysMesh::Allocate()
	{
		TASSERT(0 != m_uiMaxVertices);
		TASSERT(3 <= m_uiMaxIndices);
		TASSERT(TNULL == m_pVertexPool);
		TASSERT(TNULL == m_pIndexPool);

		auto pRender = TRender::GetSingletonWeak();
		auto pVertices = pRender->GetSystemResource(TRender::SYSRESOURCE_VFSYSVNDUV1);

		TIMPLEMENT();
	}

}
