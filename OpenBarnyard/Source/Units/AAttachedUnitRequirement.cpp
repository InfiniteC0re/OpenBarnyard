#include "pch.h"
#include "AAttachedUnitRequirement.h"
#include "AAnimatedUnit.h"
#include "AUnitType.h"
#include "Animation/AAnimatableObject.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005a7bf0, CreateObject 005a7f70
// NOTE: GetClass 005a7bf0, CreateObject 005a7f70, CreateObjectInPlace 005a7ff0
TDEFINE_CLASS( AAttachedUnitRequirement );

TPSTRING8_DECLARE( c_l_clavicle );
TPSTRING8_DECLARE( none );
TPSTRING8_DECLARE( parentbonename );
TPSTRING8_DECLARE( unittype );

AAttachedUnitRequirement::AAttachedUnitRequirement()
{
}

// $Barnyard: FUNCTION 005a7c00
// $Barnyard: FUNCTION 005a8060
AAttachedUnitRequirement::~AAttachedUnitRequirement()
{
}

// $Barnyard: FUNCTION 005a7ca0
TBOOL AAttachedUnitRequirement::Create( const PBProperties* a_pProperties )
{
	m_strParentBone = TPS8( c_l_clavicle );
	m_strUnitType   = TPS8( none );

	a_pProperties->GetOptionalPropertyValue( m_strParentBone, TPS8( parentbonename ) );
	a_pProperties->GetOptionalPropertyValue( m_strUnitType, TPS8( unittype ) );

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005a7e30
void AAttachedUnitRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TINT iNumUnits = a_pParticipants->iNumUnits;

	for ( TINT i = 0; i < iNumUnits; i++ )
	{
		AUnit* pUnit     = a_pParticipants->Get( i );
		TBOOL  bAttached = TFALSE;

		if ( pUnit->IsA( &TGetClass( AAnimatedUnit ) ) )
		{
			AAnimatedUnit* pAnimatedUnit = TSTATICCAST( AAnimatedUnit, pUnit );

			// NOTE: retail calls AModelInstance::GetBoneID( const TPString8& ) (00610da0)
			TINT iBone = pAnimatedUnit->GetAnimatableObject()->GetModelInstance()->GetSkeleton()->GetBoneID( m_strParentBone.GetString(), m_strParentBone.GetString8().Length() );

			if ( iBone >= 0 )
			{
				AUnit* pAttachedUnit = pAnimatedUnit->GetAttachedUnitByBoneId( iBone, -1 );

				if ( pAttachedUnit && TStringManager::String8CompareNoCase( pAttachedUnit->GetType()->GetName().GetString(), m_strUnitType.GetString() ) == 0 )
					bAttached = TTRUE;
			}
		}

		SetUnitAllowed( m_eCategory, i, bAttached );
	}
}
