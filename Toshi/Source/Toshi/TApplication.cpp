#include "ToshiPCH.h"
#include "TApplication.h"
#include "Toshi/Typedefs.h"
#include "Toshi/TString8.h"
#include "Toshi/TSystem.h"
#include "File/TFile.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006c16b0
TApplication::TApplication()
{
	m_Flags = 0;
}

// $Barnyard: FUNCTION 006c17c0
// $Barnyard: FUNCTION 006c16e0
TApplication::~TApplication()
{
	Destroy();
}

// $Barnyard: FUNCTION 006c17f0
TBOOL TApplication::Create( const TCHAR* appName, TINT argc, TCHAR** argv )
{
	m_pDebugConsole = new TDebugConsole;

	if ( IsConsoleEnabled() )
		m_pDebugConsole->Show( TTRUE );
	
	m_IsVerbose = TFALSE;
	m_Name = appName;

	return OnCreate( argc, argv );
}

// $Barnyard: FUNCTION 006c1760
void TApplication::Destroy()
{
	m_Flags |= TApplicationFlag_Destroyed;
}

// $Barnyard: FUNCTION 006c16f0
TBOOL TApplication::Execute()
{
	TASSERT( TApplication::IsCreated() == TTRUE );

	TBOOL bUpdateResult = TTRUE;
	while ( bUpdateResult && !IsDestroyed() )
	{
		g_oSystemManager.Update();
		bUpdateResult = OnUpdate( g_oSystemManager.GetTimer().GetDelta() );
	}

	return OnDestroy();
}

// $Barnyard: FUNCTION 006c1770
TBOOL TApplication::OnCreate( TINT argc, TCHAR** argv )
{
	m_Flags |= TApplicationFlag_Created;
	return TTRUE;
}

// $Barnyard: FUNCTION 006c1780
TBOOL TApplication::OnUpdate( TFLOAT deltaTime )
{
	return ( m_Flags & TApplicationFlag_Destroyed ) == 0;
}

// $Barnyard: FUNCTION 006c1790
TBOOL TApplication::OnDestroy()
{
	m_Name = TString8();
	return TTRUE;
}

TOSHI_NAMESPACE_END
