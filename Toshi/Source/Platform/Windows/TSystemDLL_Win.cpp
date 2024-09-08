#include "ToshiPCH.h"
#include "TSystemDLL_Win.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006da9e0
TSystemDLLWin::TSystemDLLWin()
{
	m_pDLL = TNULL;
}

TSystemDLLWin::TSystemDLLWin( const TSystemDLLWin& other )
{
	m_sFileName = other.m_sFileName;
	m_pDLL      = other.m_pDLL;
}

// $Barnyard: FUNCTION 006daa00
// $Barnyard: FUNCTION 006daa10
TSystemDLLWin::~TSystemDLLWin()
{
	Unload();
}

TBOOL TSystemDLLWin::Load( const TString8& a_sFileName )
{
#ifdef TOSHI_DYNAMIC_LINKING
	Unload();
	m_sFileName = a_sFileName;
	m_pDLL      = LoadLibraryA( m_sFileName );
	TASSERT( m_pDLL != TNULL );

	return m_pDLL != TNULL;
#else
	return TFALSE;
#endif
}

void TSystemDLLWin::Unload()
{
#ifdef TOSHI_DYNAMIC_LINKING
	if ( m_pDLL != NULL )
	{
		FreeLibrary( m_pDLL );
		m_sFileName = "";
		m_pDLL      = NULL;
	}
#endif
}

void* TSystemDLLWin::GetAddress( const TString8& a_sSymbolName )
{
	void* pAddress = GetProcAddress( GetDLL(), a_sSymbolName );
	TASSERT( pAddress != TNULL );

	return pAddress;
}

TSystemDLLWin& TSystemDLLWin::operator=( const TSystemDLLWin& other )
{
	m_sFileName = other.m_sFileName;
	m_pDLL      = other.m_pDLL;
	return *this;
}

TOSHI_NAMESPACE_END
