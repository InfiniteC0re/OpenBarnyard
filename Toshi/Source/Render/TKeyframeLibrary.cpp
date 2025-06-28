#include "ToshiPCH.h"
#include "TAnimation.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TKeyframeLibrary::TKeyframeLibrary()
{
	m_szName[ 0 ]      = '\0';
	m_iReferenceCount  = 0;
	m_iNumTranslations = 0;
	m_iNumQuaternions  = 0;
	m_iNumScales       = 0;
	m_iTranslationSize = 0;
	m_iQuaternionSize  = 0;
	m_iScaleSize       = 0;
	m_pTranslations    = TNULL;
	m_pQuaternions     = TNULL;
	m_pScales          = TNULL;
	m_pTRB             = TNULL;
}

void TKeyframeLibrary::Delete()
{
	TASSERT( m_iReferenceCount == 0 );

	if ( m_pTRB )
	{
		m_pTRB->Close();
		delete m_pTRB;
		m_pTRB = TNULL;
	}

	delete this;
}

// $Barnyard: FUNCTION 006ca600
TKeyframeLibrary* TKeyframeLibrary::CreateFromTRB( TTRB* a_pTRB, const TCHAR* a_szSymbolName )
{
	TVALIDPTR( a_pTRB );
	TVALIDPTR( a_szSymbolName );

	TRBHeader* pTRBHeader = TSTATICCAST( TRBHeader, a_pTRB->GetSymbolAddress( a_szSymbolName ) );

	return CreateFromData( pTRBHeader );
}

TKeyframeLibrary* TKeyframeLibrary::CreateFromData( TRBHeader* a_pTRBHeader )
{
	TVALIDPTR( a_pTRBHeader );

	TKeyframeLibrary* pLibrary = new TKeyframeLibrary;

	TSIZE iNameLen          = TStringManager::String8Length( a_pTRBHeader->m_szName );
	pLibrary->m_iNameLength = TUINT8( iNameLen );

	TStringManager::String8Copy( pLibrary->m_szName, a_pTRBHeader->m_szName, TMath::Min( iNameLen, sizeof( pLibrary->m_szName ) ) );
	pLibrary->m_iReferenceCount  = 0;
	pLibrary->m_SomeVector       = a_pTRBHeader->m_SomeVector;
	pLibrary->m_iNumTranslations = a_pTRBHeader->m_iNumTranslations;
	pLibrary->m_iNumQuaternions  = a_pTRBHeader->m_iNumQuaternions;
	pLibrary->m_iNumScales       = a_pTRBHeader->m_iNumScales;
	pLibrary->m_iTranslationSize = a_pTRBHeader->m_iTranslationSize;
	pLibrary->m_iQuaternionSize  = a_pTRBHeader->m_iQuaternionSize;
	pLibrary->m_iScaleSize       = a_pTRBHeader->m_iScaleSize;
	pLibrary->m_pTranslations    = a_pTRBHeader->m_pTranslations;
	pLibrary->m_pQuaternions     = a_pTRBHeader->m_pQuaternions;
	pLibrary->m_pScales          = a_pTRBHeader->m_pScales;

	return pLibrary;
}

TOSHI_NAMESPACE_END
