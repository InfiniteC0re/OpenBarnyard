#include "pch.h"
#include "AMilkComponent.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AMilkComponent );

// NOTE: inlined into CreateObject 004848e0 / CreateObjectInPlace 00484960
AMilkComponent::AMilkComponent()
    : m_fFireAccuracy( -1.0f )
    , m_fFireDelay( -1.0f )
    , m_fRechargeRate( -1.0f )
    , m_fAvoidFireAngle( -1.0f )
    , m_fFireSpeed( -1.0f )
    , m_fGrossOutShot( -1.0f )
    , m_fVerticalAngle( -1.0f )
    , m_iMaxAmmo( -1 )
{
}

// $Barnyard: FUNCTION 004849d0
// $Barnyard: FUNCTION 00484760
AMilkComponent::~AMilkComponent()
{
}

// $Barnyard: FUNCTION 00484770
TBOOL AMilkComponent::Create( const PBProperties* a_pProperties )
{
	// NOTE: retail doesn't check the value types, GetFloat converts integers like it does
	const PBPropertyValue* pRechargeRate   = a_pProperties->GetProperty( "milkrechargerate" );
	const PBPropertyValue* pFireAccuracy   = a_pProperties->GetProperty( "milkfireaccuracy" );
	const PBPropertyValue* pFireDelay      = a_pProperties->GetProperty( "milkfiredelay" );
	const PBPropertyValue* pMaxAmmo        = a_pProperties->GetProperty( "milkmaxammo" );
	const PBPropertyValue* pAvoidFireAngle = a_pProperties->GetProperty( "milkavoidfireangle" );
	const PBPropertyValue* pFireSpeed      = a_pProperties->GetProperty( "milkfirespeed" );
	const PBPropertyValue* pVerticalAngle  = a_pProperties->GetProperty( "milkverticalangle" );
	const PBPropertyValue* pGrossOutShot   = a_pProperties->GetProperty( "milkgrossoutshot" );

	if ( pVerticalAngle )
		m_fVerticalAngle = pVerticalAngle->GetFloat() * 0.017453292f;

	if ( pRechargeRate )
		m_fRechargeRate = pRechargeRate->GetFloat();

	if ( pFireAccuracy )
		m_fFireAccuracy = pFireAccuracy->GetFloat() * 0.017453292f;

	if ( pFireDelay )
		m_fFireDelay = pFireDelay->GetFloat();

	if ( pMaxAmmo )
		m_iMaxAmmo = TINT( TUINTPTR( pMaxAmmo->GetRaw() ) );

	if ( pAvoidFireAngle )
		m_fAvoidFireAngle = pAvoidFireAngle->GetFloat();

	if ( pFireSpeed )
		m_fFireSpeed = pFireSpeed->GetFloat();

	if ( pGrossOutShot )
		m_fGrossOutShot = pGrossOutShot->GetFloat();

	return TTRUE;
}
