#pragma once
#include "Toshi/TDebugConsole.h"
#include "Toshi/TString8.h"
#include "TGlobalEmitter.h"

TOSHI_NAMESPACE_START

class TDebugConsole;

enum TApplicationFlag
{
	TApplicationFlag_Created   = BITFLAG( 0 ),
	TApplicationFlag_Destroyed = BITFLAG( 1 ),
};

struct TApplicationExitEvent
{
	TBOOL m_bLockedCursor;
};

class TApplication
{
public:
	TApplication();
	virtual ~TApplication();

	virtual TBOOL OnCreate( TINT argc, TCHAR** argv );
	virtual TBOOL OnUpdate( float deltaTime );
	virtual TBOOL OnDestroy();
	virtual TBOOL IsConsoleEnabled() { return TTRUE; }

	static void OnApplicationExitEvent( void* a_pApp, const TApplicationExitEvent& a_rEvent )
	{
		TSTATICCAST( TApplication, a_pApp )->Destroy();
	}

	// Returns TTRUE if success
	TBOOL Create( const TCHAR* appName, TINT argc, TCHAR** argv );

	// Destroys the application and sets the flag
	void Destroy();

	// Returns TTRUE if the application is created
	TBOOL IsCreated() { return m_Flags & TApplicationFlag_Created; }

	// Returns TTRUE if the application is destroyed
	TBOOL IsDestroyed() { return m_Flags & TApplicationFlag_Destroyed; }

	// Sets new state
	void SetVerbose( TBOOL state ) { m_IsVerbose = state; }

	// Executes the application and starts the loop
	TBOOL Execute();

private:
	TString8                                             m_Name;       // 0x4
	TUINT32                                              m_Flags;      // 0x10
	TBOOL                                                m_IsVerbose;  // 0x14
	TGlobalListener<TApplication, TApplicationExitEvent> m_oExitEvent; // 0x1C
	TDebugConsole*                                       m_pDebugConsole;
};

TOSHI_NAMESPACE_END
