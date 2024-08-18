#include "ToshiPCH.h"
#include "TIndexPoolResourceInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_NORUNTIME( TIndexPoolResourceInterface );

TIndexPoolResourceInterface::TIndexPoolResourceInterface()
{
	m_pFactory     = TNULL;
	m_uiFlags      = 0;
	m_uiMaxIndices = 0;
	m_uiNumIndices = 0;
	m_uiLockCount  = 0;
}

void TIndexPoolResourceInterface::OnDestroy()
{
	TASSERT( 0 == m_uiLockCount );
	m_pFactory->m_uiNumPools -= 1;
}

TBOOL TIndexPoolResourceInterface::Create( TIndexFactoryResourceInterface* a_pFactory, TUINT16 a_uiMaxIndices, TUINT16 a_uiFlags )
{
	m_pFactory     = a_pFactory;
	m_uiMaxIndices = a_uiMaxIndices;
	m_uiFlags      = a_uiFlags;
	return TResource::Create();
}

TOSHI_NAMESPACE_END
