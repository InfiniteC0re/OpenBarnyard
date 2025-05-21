#include "ToshiPCH.h"
#include "TVertexFactoryResourceInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_NORUNTIME( TVertexFactoryResourceInterface );

// $Barnyard: FUNCTION 006dc0c0
TVertexFactoryResourceInterface::TVertexFactoryResourceInterface()
{
	m_uiNumPools          = 0;
	m_uiMaxStaticVertices = 0;
	m_uiFlags             = 0;
}

// $Barnyard: FUNCTION 006dc090
TBOOL TVertexFactoryResourceInterface::Create( TVertexFactoryFormat* a_pVertexFormat, TUINT16 a_uiMaxStaticVertices, TUINT32 a_uiFlags )
{
	TVALIDPTR( a_pVertexFormat );

	m_VertexFormat        = *a_pVertexFormat;
	m_uiMaxStaticVertices = a_uiMaxStaticVertices;
	m_uiFlags             = a_uiFlags;
	return TResource::Create();
}

// $Barnyard: FUNCTION 006dc1e0
TVertexPoolResourceInterface* TVertexFactoryResourceInterface::CreatePool( TUINT16 a_uiMaxStaticVertices, TUINT32 a_uiFlags )
{
	m_uiNumPools += 1;
	return TNULL;
}

TOSHI_NAMESPACE_END
