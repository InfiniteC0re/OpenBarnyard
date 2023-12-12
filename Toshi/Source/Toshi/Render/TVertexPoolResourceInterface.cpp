#include "ToshiPCH.h"
#include "TVertexPoolResourceInterface.h"

namespace Toshi {

	TVertexPoolResourceInterface::TVertexPoolResourceInterface()
	{
		m_pFactory = TNULL;
		m_uiFlags = 0;
		m_uiMaxVertices = 0;
		m_uiNumVertices = 0;
		m_uiLockCount = 0;
	}

	void TVertexPoolResourceInterface::OnDestroy()
	{
		TASSERT(0 == m_uiLockCount);
		m_pFactory->m_uiNumPools -= 1;
	}

	TBOOL TVertexPoolResourceInterface::Create(TVertexFactoryResourceInterface* a_pFactory, TUINT16 a_uiMaxVertices, TUINT16 a_uiFlags)
	{
		m_pFactory = a_pFactory;
		m_uiMaxVertices = a_uiMaxVertices;
		m_uiFlags = a_uiFlags;
		return TResource::Create();
	}

	TVertexPoolResourceInterface::LockBuffer::LockBuffer()
	{
		m_uiNumStreams = 0;
		m_uiOffset = 0;
		m_apStreams[0] = TNULL;
		m_apStreams[1] = TNULL;
		m_apStreams[2] = TNULL;
		m_apStreams[3] = TNULL;
		m_apStreams[4] = TNULL;
		m_apStreams[5] = TNULL;
		m_apStreams[6] = TNULL;
		m_apStreams[7] = TNULL;
	}

}
