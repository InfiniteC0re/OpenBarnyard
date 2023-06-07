#include "ToshiPCH.h"
#include "TSysMesh.h"
#include "Toshi/Render/TRender.h"

namespace Toshi {

	TSysMesh::TSysMesh()
	{
		m_uiFlags = 0;
		m_uiMaxVertices = 0;
		m_uiMaxIndices = 0;
		m_pVertexPool = TNULL;
		m_pIndexPool = TNULL;
	}

	TBOOL TSysMesh::Validate()
	{
		if (!m_State.IsSet(State::Validated))
			m_State.Set(State::Validated);

		return TTRUE;
	}

	void TSysMesh::Invalidate()
	{
		if (m_pVertexPool != TNULL && m_pIndexPool != TNULL)
			Deallocate();

		if (m_State.IsSet(State::Validated))
			m_State.Unset(State::Validated);
	}

	TBOOL TSysMesh::Create(FLAGS uiFlags, uint16_t uiMaxVertices, uint16_t uiMaxIndices)
	{
		TASSERT(TFALSE == IsCreated());

		m_uiFlags = uiFlags;
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

	TBOOL TSysMesh::Lock(TLockBuffer* pLockBuffer)
	{
		TASSERT(!(m_uiFlags & FLAGS_LOCKED));
		TIMPLEMENT();

		return TTRUE;
	}

	void TSysMesh::Unlock(uint16_t uiNumVertices, uint16_t uiNumIndices)
	{
		TASSERT(m_uiFlags & FLAGS_LOCKED);
		TASSERT(uiNumVertices >= 3);
		TASSERT(uiNumIndices >= 3);
		
		TIMPLEMENT();
	}

	TResource* TSysMesh::GetVertexPool() const
	{
		return m_pVertexPool;
	}

	TResource* TSysMesh::GetIndexPool() const
	{
		return m_pIndexPool;
	}

	TBOOL TSysMesh::Allocate()
	{
		TASSERT(0 != m_uiMaxVertices);
		TASSERT(3 <= m_uiMaxIndices);
		TASSERT(TNULL == m_pVertexPool);
		TASSERT(TNULL == m_pIndexPool);

		auto pRender = TRender::GetSingletonWeak();
		auto pVertices = pRender->GetSystemResource(TRender::SYSRESOURCE_VFSYSVNDUV1);

		TASSERT(TFALSE, "Not used in De Blob on Windows");
		TIMPLEMENT();

		return true;
	}

	void TSysMesh::Deallocate()
	{
		if (m_pVertexPool != TNULL)
		{
			TRender::GetSingletonWeak()->DestroyResource(m_pVertexPool);
			m_pVertexPool = TNULL;
		}

		if (m_pIndexPool != TNULL)
		{
			TRender::GetSingletonWeak()->DestroyResource(m_pIndexPool);
			m_pIndexPool = TNULL;
		}

		m_uiFlags = 0;
		m_uiMaxVertices = 0;
		m_uiMaxIndices = 0;
	}

}
