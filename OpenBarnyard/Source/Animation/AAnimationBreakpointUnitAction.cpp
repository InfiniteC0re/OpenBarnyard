#include "pch.h"
#include "AAnimationBreakpointUnitAction.h"
#include "AAnimatableObject.h"
#include "Units/AUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00585070, CreateObject 00585340, CreateObjectInPlace 005853c0 (the constructor is inlined in both)
TDEFINE_CLASS( AAnimationBreakpointUnitAction );

TPSTRING8_DECLARE( Mode );
TPSTRING8_DECLARE( pickup_bp );
TPSTRING8_DECLARE( hd_throw_bp );
TPSTRING8_DECLARE( start_bp );
TPSTRING8_DECLARE( end_bp );
TPSTRING8_DECLARE( start_climb_getoff_bp );
TPSTRING8_DECLARE( end_climb_getoff_bp );
TPSTRING8_DECLARE( attach_bp );
TPSTRING8_DECLARE( detach_bp );
TPSTRING8_DECLARE( putdown_bp );

// 0x007740f0
static TPString8* s_apActionNames[ AAnimationBreakpointUnitAction::ACTION_NUMOF ] = {
	&TPS8( pickup_bp ),
	&TPS8( hd_throw_bp ),
	&TPS8( start_bp ),
	&TPS8( end_bp ),
	&TPS8( start_climb_getoff_bp ),
	&TPS8( end_climb_getoff_bp ),
	&TPS8( attach_bp ),
	&TPS8( detach_bp ),
	&TPS8( putdown_bp ),
};

AAnimationBreakpointUnitAction::Listener AAnimationBreakpointUnitAction::ms_oListeners = {
	&AAnimationBreakpointUnitAction::ms_oListeners,
	&AAnimationBreakpointUnitAction::ms_oListeners,
	TNULL,
	TNULL
};

AAnimationBreakpointUnitAction::Listener AAnimationBreakpointUnitAction::ms_oOtherListeners = {
	&AAnimationBreakpointUnitAction::ms_oOtherListeners,
	&AAnimationBreakpointUnitAction::ms_oOtherListeners,
	TNULL,
	TNULL
};

AAnimationBreakpointUnitAction::AAnimationBreakpointUnitAction()
{
}

// $Barnyard: FUNCTION 00585450
// $Barnyard: FUNCTION 00585430
AAnimationBreakpointUnitAction::~AAnimationBreakpointUnitAction()
{
}

// $Barnyard: FUNCTION 005851b0
TBOOL AAnimationBreakpointUnitAction::Create( const PBProperties* a_pProperties )
{
	TBOOL bResult = AAnimationBreakpoint::Create( a_pProperties );

	TPString8 strMode;
	a_pProperties->GetOptionalPropertyValue( strMode, TPS8( Mode ) );

	TINT i = 0;
	for ( ; i < ACTION_NUMOF; i++ )
	{
		if ( strMode == *s_apActionNames[ i ] )
			break;
	}

	m_eAction = i;

	return bResult;
}

// $Barnyard: FUNCTION 00585250
void AAnimationBreakpointUnitAction::Execute( AAnimatableObject* a_pObject, TFLOAT a_fTime )
{
	AUnit* pOwner = TSTATICCAST( AUnit, a_pObject->GetOwner() );

	if ( pOwner )
	{
		Event oEvent( m_eAction, pOwner->GetSelfProxy() );
		ThrowEvent( &oEvent );
	}
	else
	{
		Event oEvent( m_eAction, TNULL );
		ThrowEvent( &oEvent );
	}
}

// $Barnyard: FUNCTION 005850d0
void AAnimationBreakpointUnitAction::ThrowEvent( Event* a_pEvent )
{
	Listener* pListener = ms_oListeners.pNext;

	while ( pListener != &ms_oListeners )
	{
		Listener* pNext = pListener->pNext;
		pListener->pCallback( pListener->pCaller, a_pEvent );
		pListener = pNext;
	}
}

// $Barnyard: FUNCTION 00585140
AAnimationBreakpointUnitAction::Event::Event( ACTION a_eAction, TRef<AUnitProxy> a_pUnitProxy )
    : eAction( a_eAction ), pUnitProxy( a_pUnitProxy )
{
}

// $Barnyard: FUNCTION 00585080
// $Barnyard: FUNCTION 005850b0
AAnimationBreakpointUnitAction::Event::~Event()
{
}

// $Barnyard: FUNCTION 004c7730
TRef<AUnitProxy> AAnimationBreakpointUnitAction::Event::GetUnitProxy() const
{
	return pUnitProxy;
}

// $Barnyard: FUNCTION 00585120
void AAnimationBreakpointUnitAction::Event::Throw()
{
	ThrowEvent( this );
	ThrowOtherEvent( this );
}

// $Barnyard: FUNCTION 006cbf50
void AAnimationBreakpointUnitAction::ThrowOtherEvent( Event* a_pEvent )
{
	Listener* pListener = ms_oOtherListeners.pNext;

	while ( pListener != &ms_oOtherListeners )
	{
		Listener* pNext = pListener->pNext;
		pListener->pCallback( pListener->pCaller, a_pEvent );
		pListener = pNext;
	}
}
