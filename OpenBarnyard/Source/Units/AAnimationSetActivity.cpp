#include "pch.h"
#include "AAnimationSetActivity.h"
#include "AAnimatedUnit.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectType.h"
#include "Animation/ANamedAnimationSet.h"
#include "Animation/AAlienAnimController.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b30a0, CreateObject 005b3280
// NOTE: GetClass 005b30a0, CreateObject 005b3280, CreateObjectInPlace 005b32f0
TDEFINE_CLASS( AAnimationSetActivity );

TPSTRING8_DECLARE( animationset );

AAnimationSetActivity::AAnimationSetActivity()
{
}

// $Barnyard: FUNCTION 005b30e0
// $Barnyard: FUNCTION 005b3360
AAnimationSetActivity::~AAnimationSetActivity()
{
}

// $Barnyard: FUNCTION 005b30b0
TBOOL AAnimationSetActivity::Create( const PBProperties* a_pProperties )
{
	a_pProperties->GetOptionalPropertyValue( m_strAnimationSet, TPS8( animationset ) );

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b3150
AInteractionActivity::RESULT AAnimationSetActivity::Update( AInteractionSequence* a_pSequence )
{
	for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
	{
		AAnimatableObject* pAnimObj = TSTATICCAST( AAnimatedUnit, it->GetOwner() )->GetAnimatableObject();

		// NOTE: inlined AAnimatableObjectType::FindAnimationSetOrDefault, the named animation set of the type or its default one
		ANamedAnimationSetRef pAnimationSet = pAnimObj->GetObjectType()->FindAnimationSet( m_strAnimationSet );

		if ( !pAnimationSet.Get() )
			pAnimationSet = pAnimObj->GetObjectType()->GetDefaultAnimationSet();

		pAnimObj->GetToshiAnimInterface().SetAnimationSet( pAnimationSet );

		AAnimController* pController = pAnimObj->FindAnimController( &TGetClass( ACharacterAnimController ) );

		if ( pController || ( pController = pAnimObj->FindAnimController( &TGetClass( AAlienAnimController ) ) ) )
		{
			TSTATICCAST( ACharacterAnimController, pController )->SetAnimationSet( &m_strAnimationSet );
			pAnimObj->KillAllAnimations();
		}
	}

	return RESULT_DONE;
}
