#pragma once
#include "ASimAnimModelHelper.h"

#include <Toshi/TClass.h>
#include <Toshi/TTask.h>

class ASimAnimModelHelperManager
    : public Toshi::TTask
    , public Toshi::TSingleton<ASimAnimModelHelperManager>
{
public:
	TDECLARE_CLASS( ASimAnimModelHelperManager, Toshi::TTask );

	static constexpr TINT MAX_NUM_HELPERS = 2;

public:
	ASimAnimModelHelperManager();
	~ASimAnimModelHelperManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;

	// Adds helper to the list and returns it's index in the array
	TINT AddModelHelper( ASimAnimModelHelper* a_pModelHelper );

private:
	TINT                 m_iNumHelpers;
	ASimAnimModelHelper* m_apHelpers[ MAX_NUM_HELPERS ];
};
