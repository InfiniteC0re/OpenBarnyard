#include "ToshiPCH.h"
#include "TGUISineFunction.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TGUISineFunction );

// NOTE: inlined into CreateObject and CreateObjectInPlace in Nicktoons
// $NicktoonsPS2: FUNCTION 00363cd0
// $NicktoonsPS2: FUNCTION 00363d38
// $JPOG: FUNCTION TGuiInterface.dll+00007190
TGUISineFunction::TGUISineFunction()
{
}

// $JPOG: FUNCTION TGuiInterface.dll+00007150
TGUISineFunction::TGUISineFunction( TFLOAT a_fFrequency, TFLOAT a_fPhase, TFLOAT a_fScale, TFLOAT a_fOffset )
    : TGUIFunction( a_fFrequency, a_fPhase, a_fScale, a_fOffset )
{
}

// $JPOG: FUNCTION TGuiInterface.dll+000070d0
TGUISineFunction::TGUISineFunction( const TGUISineFunction& a_rcOther )
    : TGUIFunction( a_rcOther )
{
}

// $NicktoonsPS2: FUNCTION 00364430
// $JPOG: FUNCTION TGuiInterface.dll+000070c0
TGUISineFunction::~TGUISineFunction()
{
}

// $NicktoonsPS2: FUNCTION 00364458
// $JPOG: FUNCTION TGuiInterface.dll+00007120
TGUISineFunction* TGUISineFunction::Clone() const
{
	return new TGUISineFunction( *this );
}

// $NicktoonsPS2: FUNCTION 003644c8
// $JPOG: FUNCTION TGuiInterface.dll+00007100
TFLOAT TGUISineFunction::ComputeNormalised( TFLOAT a_fX ) const
{
	return TMath::Sin( a_fX * TMath::PI );
}

TOSHI_NAMESPACE_END
