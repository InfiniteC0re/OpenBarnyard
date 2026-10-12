#pragma once
#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>

//-----------------------------------------------------------------------------
// Purpose: Owns and updates ASplashManager.
// Singleton at 0x007b5f10, vtable 0x0073acd4.
//-----------------------------------------------------------------------------
class AFXManager
    : public Toshi::TTask
    , public Toshi::TSingleton<AFXManager>
{
public:
	TDECLARE_CLASS( AFXManager, Toshi::TTask );

public:
	AFXManager();
	~AFXManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL OnCreate() OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnDestroy() OVERRIDE;
};

TSTATICASSERT( sizeof( AFXManager ) == 0x20 );
