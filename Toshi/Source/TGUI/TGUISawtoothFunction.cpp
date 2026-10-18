#include "ToshiPCH.h"
#include "TGUISawtoothFunction.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TGUISawtoothFunction );

// NOTE: inlined into CreateObject and CreateObjectInPlace in Nicktoons
// $NicktoonsPS2: FUNCTION 00363db0
// $NicktoonsPS2: FUNCTION 00363e18
// $JPOG: FUNCTION TGuiInterface.dll+00007080
TGUISawtoothFunction::TGUISawtoothFunction()
{
}

// $JPOG: FUNCTION TGuiInterface.dll+00007040
TGUISawtoothFunction::TGUISawtoothFunction( TFLOAT a_fFrequency, TFLOAT a_fPhase, TFLOAT a_fScale, TFLOAT a_fOffset )
    : TGUIFunction( a_fFrequency, a_fPhase, a_fScale, a_fOffset )
{
}

// $JPOG: FUNCTION TGuiInterface.dll+00006fb0
TGUISawtoothFunction::TGUISawtoothFunction( const TGUISawtoothFunction& a_rcOther )
    : TGUIFunction( a_rcOther )
{
}

// $NicktoonsPS2: FUNCTION 003644e8
// $JPOG: FUNCTION TGuiInterface.dll+00006fa0
TGUISawtoothFunction::~TGUISawtoothFunction()
{
}

// $NicktoonsPS2: FUNCTION 00364510
// $JPOG: FUNCTION TGuiInterface.dll+00007010
TGUISawtoothFunction* TGUISawtoothFunction::Clone() const
{
	return new TGUISawtoothFunction( *this );
}

// $NicktoonsPS2: FUNCTION 00364580
// $JPOG: FUNCTION TGuiInterface.dll+00006fe0
TFLOAT TGUISawtoothFunction::ComputeNormalised( TFLOAT a_fX ) const
{
	TFLOAT fValue = fmodf( a_fX, 1.0f );
	return fValue + fValue - 1.0f;
}

TOSHI_NAMESPACE_END
