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

TApplication::TApplication()
{
	m_Flags = 0;
}

TApplication::~TApplication()
{
	Destroy();
}

TBOOL TApplication::Create( const TCHAR* appName, TINT argc, TCHAR** argv )
{
	m_oExitEvent.Connect( this, OnApplicationExitEvent );
	m_Name = appName;

	m_pDebugConsole = new TDebugConsole;

	if ( IsConsoleEnabled() )
		m_pDebugConsole->Show( TTRUE );

	return OnCreate( argc, argv );
}

void TApplication::Destroy()
{
	m_Flags |= TApplicationFlag_Destroyed;
}

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

TBOOL TApplication::OnCreate( TINT argc, TCHAR** argv )
{
	m_Flags |= TApplicationFlag_Created;
	return TTRUE;
}

TBOOL TApplication::OnUpdate( TFLOAT deltaTime )
{
	return ( m_Flags & TApplicationFlag_Destroyed ) == 0;
}

TBOOL TApplication::OnDestroy()
{
	m_oExitEvent.Disconnect();
	return TTRUE;
}

TOSHI_NAMESPACE_END
