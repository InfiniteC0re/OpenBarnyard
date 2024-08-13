#include "ToshiPCH.h"
#include "TAnimation.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TKeyframeLibraryInstance::TKeyframeLibraryInstance()
{
	m_pLibrary		= TNULL;
	m_iTBaseIndex	= 0;
	m_iQBaseIndex	= 0;
	m_iSBaseIndex	= 0;
	m_iTKeyCount	= 0;
	m_iQKeyCount	= 0;
	m_iSKeyCount	= 0;
	m_pTranslations = TNULL;
	m_pQuaternions	= TNULL;
	m_pScales		= TNULL;
}

TKeyframeLibrary* TKeyframeLibraryInstance::Create( TKeyframeLibrary* a_pLibrary )
{
	return CreateEx( a_pLibrary, a_pLibrary->m_iNumTranslations, a_pLibrary->m_iNumQuaternions, a_pLibrary->m_iNumScales, 0, 0, 0 );
}

TKeyframeLibrary* TKeyframeLibraryInstance::CreateEx( TKeyframeLibrary* a_pLibrary, TINT a_iTKeyCount, TINT a_iQKeyCount, TINT a_iSKeyCount, TINT a_iTBaseIndex, TINT a_iQBaseIndex, TINT a_iSBaseIndex )
{
	TKeyframeLibrary* pOldLibrary = m_pLibrary;

	Destroy();
	m_pLibrary = a_pLibrary;

	if ( a_pLibrary != TNULL )
	{
		a_pLibrary->IncreaseRefCount();

		m_iTKeyCount	= a_iTKeyCount;
		m_iQKeyCount	= a_iQKeyCount;
		m_iTBaseIndex	= a_iTBaseIndex;
		m_iQBaseIndex	= a_iQBaseIndex;
		m_iSBaseIndex	= a_iSBaseIndex;
		m_iSKeyCount	= a_iSKeyCount;
		m_pTranslations = a_pLibrary->m_pTranslations + a_iTBaseIndex;
		m_pQuaternions	= a_pLibrary->m_pQuaternions + a_iQBaseIndex;
		m_pScales		= a_pLibrary->m_pScales + a_iSBaseIndex;
	}

	return pOldLibrary;
}

void TKeyframeLibraryInstance::Destroy()
{
	if ( m_pLibrary != TNULL )
	{
		m_pLibrary->DecreaseRefCount();
		m_pLibrary = TNULL;
	}
}

TOSHI_NAMESPACE_END
