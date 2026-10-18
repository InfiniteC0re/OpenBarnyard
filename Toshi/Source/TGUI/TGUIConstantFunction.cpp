#include "ToshiPCH.h"
#include "TGUIConstantFunction.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TGUIConstantFunction );

// NOTE: inlined into CreateObject and CreateObjectInPlace in Nicktoons
// $NicktoonsPS2: FUNCTION 00363bf0
// $NicktoonsPS2: FUNCTION 00363c58
// $JPOG: FUNCTION TGuiInterface.dll+00007290
TGUIConstantFunction::TGUIConstantFunction()
{
}

// $JPOG: FUNCTION TGuiInterface.dll+00007250
TGUIConstantFunction::TGUIConstantFunction( TFLOAT a_fFrequency, TFLOAT a_fPhase, TFLOAT a_fScale, TFLOAT a_fOffset )
    : TGUIFunction( a_fFrequency, a_fPhase, a_fScale, a_fOffset )
{
}

// $JPOG: FUNCTION TGuiInterface.dll+000071e0
TGUIConstantFunction::TGUIConstantFunction( const TGUIConstantFunction& a_rcOther )
    : TGUIFunction( a_rcOther )
{
}

// $NicktoonsPS2: FUNCTION 00364388
// $JPOG: FUNCTION TGuiInterface.dll+000071d0
TGUIConstantFunction::~TGUIConstantFunction()
{
}

// $NicktoonsPS2: FUNCTION 003643b0
// $JPOG: FUNCTION TGuiInterface.dll+00007220
TGUIConstantFunction* TGUIConstantFunction::Clone() const
{
	return new TGUIConstantFunction( *this );
}

// $NicktoonsPS2: FUNCTION 00364420
// $JPOG: FUNCTION TGuiInterface.dll+00007210
TFLOAT TGUIConstantFunction::ComputeNormalised( TFLOAT a_fX ) const
{
	// NOTE: the result is always 0, Compute returns the offset
	return 0.0f;
}

TOSHI_NAMESPACE_END
