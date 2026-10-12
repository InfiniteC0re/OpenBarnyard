#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TSingleton.h>
#include <Math/TRandom.h>

class ARandom : public Toshi::TObject
    , public Toshi::TSingleton<ARandom>
{
public:
	TDECLARE_CLASS( ARandom, Toshi::TObject );

public:
	// $Barnyard: FUNCTION 004029d0
	ARandom()
	{
	}

	// $Barnyard: FUNCTION 00402a30
	// $Barnyard: FUNCTION 00402af0
	~ARandom()
	{
	}

	// NOTE: retail has no wrapper methods, callers use m_oRandom directly

	// Helpers of the weighted random pickers with integer weights (see ACowTippingMiniGameState::PickWeighted)

	// $Barnyard: FUNCTION 00401120
	static TINT GetRandomWeight( Toshi::TRandom* a_pRandom, TINT a_iMaxWeight ) { return a_pRandom->GetInt( a_iMaxWeight ); }

	// $Barnyard: FUNCTION 00401160
	static TBOOL IsWeightZero( TINT a_iWeight ) { return a_iWeight == 0; }

	Toshi::TRandom m_oRandom;
};
