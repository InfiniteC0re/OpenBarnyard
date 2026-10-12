#include "pch.h"
#include "AAnimatableRequirement.h"
#include "AAnimatedUnit.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/ANamedAnimationSet.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005a7900, CreateObject 005a7af0
// NOTE: GetClass 005a7900, CreateObject 005a7af0, CreateObjectInPlace 005a7b60
TDEFINE_CLASS( AAnimatableRequirement );

TPSTRING8_DECLARE( animation );

// $Barnyard: FUNCTION 005a7a80
// $Barnyard: FUNCTION 005a7bd0
AAnimatableRequirement::~AAnimatableRequirement()
{
}

// $Barnyard: FUNCTION 005a7910
TBOOL AAnimatableRequirement::Create( const PBProperties* a_pProperties )
{
	a_pProperties->GetOptionalPropertyValue( m_strAnimation, TPS8( animation ) );

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005a7a00
void AAnimatableRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TINT iNumUnits = a_pParticipants->iNumUnits;

	for ( TINT i = 0; i < iNumUnits; i++ )
	{
		AUnit* pUnit    = a_pParticipants->Get( i );
		TBOOL  bAllowed = TFALSE;

		if ( pUnit->GetClass()->IsA( &TGetClass( AAnimatedUnit ) ) == TTRUE )
		{
			// NOTE: AAnimatableObject::HasAnimation, does the animation set of the object have the animation?
			AAnimatableObject* pAnimObj = TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject();
			bAllowed                    = pAnimObj->GetToshiAnimInterface().GetAnimationSet()->GetNamedAnimation( m_strAnimation ) != TNULL;
		}

		SetUnitAllowed( m_eCategory, i, bAllowed );
	}
}
