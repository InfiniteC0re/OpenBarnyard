#include "pch.h"
#include "AAnimationBreakpointAnimation.h"
#include "AAnimatableObject.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00583ab0, CreateObject 00583c60, CreateObjectInPlace 00583ce0 (the constructor is inlined in both)
TDEFINE_CLASS( AAnimationBreakpointAnimation );

AAnimationBreakpointAnimation::AAnimationBreakpointAnimation()
    : m_eMode( MODE_PLAY_ANIMATION )
{
}

// $Barnyard: FUNCTION 00583d70
// $Barnyard: FUNCTION 00583d50
AAnimationBreakpointAnimation::~AAnimationBreakpointAnimation()
{
}

TPSTRING8_DECLARE( Mode );
TPSTRING8_DECLARE( PlayAnimation );
TPSTRING8_DECLARE( PlayChildAnimation );
TPSTRING8_DECLARE( PlayParentAnimation );
TPSTRING8_DECLARE( ModelName );
TPSTRING8_DECLARE( BoneName );
TPSTRING8_DECLARE( Animation );

// $Barnyard: FUNCTION 00583b70
TBOOL AAnimationBreakpointAnimation::Create( const PBProperties* a_pProperties )
{
	TBOOL bResult = AAnimationBreakpoint::Create( a_pProperties );

	TPString8 strMode;
	a_pProperties->GetOptionalPropertyValue( strMode, TPS8( Mode ) );

	if ( strMode == TPS8( PlayAnimation ) )
		m_eMode = MODE_PLAY_ANIMATION;
	else if ( strMode == TPS8( PlayChildAnimation ) )
		m_eMode = MODE_PLAY_CHILD_ANIMATION;
	else if ( strMode == TPS8( PlayParentAnimation ) )
		m_eMode = MODE_PLAY_PARENT_ANIMATION;

	if ( m_eMode == MODE_PLAY_CHILD_ANIMATION )
	{
		a_pProperties->GetOptionalPropertyValue( m_strModelName, TPS8( ModelName ) );
		a_pProperties->GetOptionalPropertyValue( m_strBoneName, TPS8( BoneName ) );
	}

	a_pProperties->GetOptionalPropertyValue( m_strAnimation, TPS8( Animation ) );

	return bResult;
}

// $Barnyard: FUNCTION 00583b20
void AAnimationBreakpointAnimation::Execute( AAnimatableObject* a_pObject, TFLOAT a_fTime )
{
	AAnimatableObject* pTarget;

	switch ( m_eMode )
	{
		case MODE_PLAY_ANIMATION:
			pTarget = a_pObject;
			break;
		case MODE_PLAY_CHILD_ANIMATION:
			pTarget = FindChild( a_pObject );
			break;
		case MODE_PLAY_PARENT_ANIMATION:
			pTarget = a_pObject->GetParentObject();
			break;
		default:
			return;
	}

	if ( pTarget )
		pTarget->PlayAnimation( m_strAnimation );
}

// $Barnyard: FUNCTION 00583ac0
AAnimatableObject* AAnimationBreakpointAnimation::FindChild( AAnimatableObject* a_pObject )
{
	if ( !m_strModelName.IsEmpty() )
		return a_pObject->FindAttachedObjectOfType( m_strModelName );

	if ( !m_strBoneName.IsEmpty() )
		return a_pObject->FindAttachedObject( m_strBoneName );

	return TNULL;
}
