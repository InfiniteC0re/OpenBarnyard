#include "ToshiPCH.h"
#include "TVertexFactoryResourceInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi {

	TDEFINE_CLASS_NORUNTIME(TVertexFactoryResourceInterface);

	TVertexFactoryResourceInterface::TVertexFactoryResourceInterface()
	{
		m_uiNumPools = 0;
		m_uiMaxStaticVertices = 0;
		m_uiFlags = 0;
	}

	TBOOL TVertexFactoryResourceInterface::Create(TVertexFactoryFormat* a_pVertexFormat, TUINT16 a_uiMaxStaticVertices, TUINT32 a_uiFlags)
	{
		TVALIDPTR(a_pVertexFormat);

		m_VertexFormat = *a_pVertexFormat;
		m_uiMaxStaticVertices = a_uiMaxStaticVertices;
		m_uiFlags = a_uiFlags;
		return TResource::Create();
	}

	TVertexPoolResourceInterface* TVertexFactoryResourceInterface::CreatePool(TUINT16 a_uiMaxStaticVertices, TUINT32 a_uiFlags)
	{
		m_uiNumPools += 1;
		return TNULL;
	}

}
