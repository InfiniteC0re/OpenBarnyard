#include "pch.h"
#include "AAnimationBreakpoint.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AAnimationBreakpoint );

// $Barnyard: FUNCTION 0057fcf0
AAnimationBreakpoint::AAnimationBreakpoint()
    : m_fTime( 0.0f )
{
}

AAnimationBreakpoint::~AAnimationBreakpoint()
{
}

// $Barnyard: FUNCTION 0057fd30
TBOOL AAnimationBreakpoint::SetTime( TFLOAT a_fTime )
{
	if ( IsTimed() )
		m_fTime = a_fTime;

	return TTRUE;
}

TPSTRING8_DECLARE( Time );

// $Barnyard: FUNCTION 0057fd10
TBOOL AAnimationBreakpoint::Create( const PBProperties* a_pProperties )
{
	if ( IsTimed() )
		a_pProperties->GetOptionalFloatOrIntValue( m_fTime, TPS8( Time ) );

	return TTRUE;
}

// $Barnyard: FUNCTION 0057fd70
AAnimationBreakpoint* AAnimationBreakpoint::CreateBreakpoint( const TPString8& a_rcType, TBOOL a_bTimed )
{
	// Breakpoint classes are named AAnimationBreakpoint<Type>
	TString8 strClassName;
	strClassName.Format( "AAnimationBreakpoint%s", a_rcType.GetString8().GetString() );

	const TClass* pClass = TClass::Find( strClassName, &TGetClass( AAnimationBreakpoint ) );

	if ( !pClass )
		return TNULL;

	AAnimationBreakpoint* pBreakpoint = TSTATICCAST( AAnimationBreakpoint, pClass->CreateObject() );

	if ( !a_bTimed )
		pBreakpoint->SetUntimed();

	return pBreakpoint;
}
