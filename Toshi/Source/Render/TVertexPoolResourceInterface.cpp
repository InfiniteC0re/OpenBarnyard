#include "ToshiPCH.h"
#include "TVertexPoolResourceInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_NORUNTIME( TVertexPoolResourceInterface );

// $Barnyard: FUNCTION 006daae0
TVertexPoolResourceInterface::TVertexPoolResourceInterface()
{
	m_pFactory      = TNULL;
	m_uiFlags       = 0;
	m_uiMaxVertices = 0;
	m_uiNumVertices = 0;
	m_uiLockCount   = 0;
}

// $Barnyard: FUNCTION 006daa90
void TVertexPoolResourceInterface::OnDestroy()
{
	TASSERT( 0 == m_uiLockCount );
	m_pFactory->m_uiNumPools -= 1;
}

// $Barnyard: FUNCTION 006dab20
TBOOL TVertexPoolResourceInterface::Create( TVertexFactoryResourceInterface* a_pFactory, TUINT16 a_uiMaxVertices, TUINT16 a_uiFlags )
{
	m_pFactory      = a_pFactory;
	m_uiMaxVertices = a_uiMaxVertices;
	m_uiFlags       = a_uiFlags;
	return TResource::Create();
}

TVertexPoolResourceInterface::LockBuffer::LockBuffer()
{
	uiNumStreams   = 0;
	uiOffset       = 0;
	apStreams[ 0 ] = TNULL;
	apStreams[ 1 ] = TNULL;
	apStreams[ 2 ] = TNULL;
	apStreams[ 3 ] = TNULL;
	apStreams[ 4 ] = TNULL;
	apStreams[ 5 ] = TNULL;
	apStreams[ 6 ] = TNULL;
	apStreams[ 7 ] = TNULL;
}

TOSHI_NAMESPACE_END
