#include "pch.h"
#include "AAttachedAnimController.h"
#include "AAnimatableObject.h"
#include "AAnimatableObjectManager.h"
#include "Units/AAnimatedUnit.h"
#include "Units/AUnitType.h"
#include "Units/AInteractionComponent.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00585f20, CreateObject 00586040, CreateObjectInPlace 005860b0
TDEFINE_CLASS( AAttachedAnimController );

AAttachedAnimController::AAttachedAnimController()
{
}

// $Barnyard: FUNCTION 00586120
// $Barnyard: FUNCTION 00585f30
AAttachedAnimController::~AAttachedAnimController()
{
}

// $Barnyard: FUNCTION 00586140
TBOOL AAttachedAnimController::OnCreate()
{
	SetAnimationSet( TNULL );
	return TTRUE;
}

TPSTRING8_DECLARE( c_head );

// $Barnyard: FUNCTION 00586160
void AAttachedAnimController::OnUpdate( TFLOAT a_fDeltaTime )
{
	AAnimatedUnit*     pUnit         = TSTATICCAST( AAnimatedUnit, m_pOwner );
	AAnimatableObject* pParentObject = pUnit->GetAnimatableObject()->GetParentObject();
	AAnimatedUnit*     pParentUnit   = pParentObject ? TSTATICCAST( AAnimatedUnit, pParentObject->GetOwner() ) : TNULL;
	AAnimatedUnit*     pOnHeadUnit   = TNULL;

	if ( pParentUnit )
	{
		// Look for this unit on the head of the parent
		AAnimatableObject* pParentAnimObject = pParentUnit->GetAnimatableObject();
		TINT               iNumOnHead        = AAnimatableObjectManager::GetSingleton()->GetNumAttachedObjects( pParentAnimObject, TPS8( c_head ) );

		for ( TINT i = 0; i < iNumOnHead; i++ )
		{
			AAnimatableObject* pAttached = AAnimatableObjectManager::GetSingleton()->GetAttachedObjectByBoneName( pParentAnimObject, TPS8( c_head ), i );
			pOnHeadUnit                  = TSTATICCAST( AAnimatedUnit, pAttached->GetOwner() );

			if ( pOnHeadUnit == pUnit )
				break;
		}
	}

	if ( pUnit == pOnHeadUnit && pOnHeadUnit && pUnit )
	{
		// Sitting on the head of the parent
		if ( m_bPlayingCustomAnim )
		{
			if ( IsMovementAnimPlaying() )
				m_bPlayingCustomAnim = TFALSE;

			if ( m_bPlayingCustomAnim )
				return;
		}

		UpdateOnHeadAnimation( a_fDeltaTime );
		return;
	}

	ACharacterAnimController::OnUpdate( a_fDeltaTime );
}

TPSTRING8_DECLARE( onhead_idle );
TPSTRING8_DECLARE( otis_onhead_idle );

// $Barnyard: FUNCTION 00585f40
TBOOL AAttachedAnimController::IsMovementAnimPlaying()
{
	AToshiAnimationInterface& rAnimInterface = m_pAnimatableObject->GetToshiAnimInterface();

	if ( rAnimInterface.IsAnimPlaying( TPS8( onhead_idle ), TFALSE ) || rAnimInterface.IsAnimPlaying( TPS8( otis_onhead_idle ), TFALSE ) )
		return TTRUE;

	return ACharacterAnimController::IsMovementAnimPlaying();
}

TPSTRING8_DECLARE( talk_01 );
TPSTRING8_DECLARE( listen_01 );
TPSTRING8_DECLARE( otis );
TPSTRING8_DECLARE( bk_ftdn );
TPSTRING8_DECLARE( bk_slow );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( walk_lp );
TPSTRING8_DECLARE( onhead_walk );
TPSTRING8_DECLARE( onhead_movingfast );

// $Barnyard: FUNCTION 00586250
void AAttachedAnimController::UpdateOnHeadAnimation( TFLOAT a_fDeltaTime )
{
	AAnimatedUnit*     pUnit         = TSTATICCAST( AAnimatedUnit, m_pOwner );
	AAnimatableObject* pParentObject = pUnit->GetAnimatableObject()->GetParentObject();
	AAnimatedUnit*     pParentUnit   = pParentObject ? TSTATICCAST( AAnimatedUnit, pParentObject->GetOwner() ) : TNULL;

	// Don't interrupt dialogs
	if ( m_pAnimatableObject->IsAnimPlaying( TPS8( talk_01 ), 0.0f ) ) return;
	if ( m_pAnimatableObject->IsAnimPlaying( TPS8( listen_01 ), 0.0f ) ) return;
	if ( pParentUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( talk_01 ), 0.0f ) ) return;
	if ( pParentUnit->GetAnimatableObject()->IsAnimPlaying( TPS8( listen_01 ), 0.0f ) ) return;

	// Nor interactions
	AInteractionComponent* pInteraction = TSTATICCAST( AInteractionComponent, pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) );

	if ( pInteraction && pInteraction->GetSequence() )
		return;

	AToshiAnimationInterface& rAnimInterface       = pUnit->GetAnimatableObject()->GetToshiAnimInterface();
	AToshiAnimationInterface& rParentAnimInterface = pParentUnit->GetAnimatableObject()->GetToshiAnimInterface();

	TBOOL     bOnOtis = pParentUnit->GetType()->GetName() == TPS8( otis );
	TPString8 strIdleAnim;

	if ( bOnOtis )
		strIdleAnim = TPS8( otis_onhead_idle );
	else
		strIdleAnim = TPS8( onhead_idle );

	// Is the parent standing still?
	AToshiAnimationRef* pParentAnimRef = rParentAnimInterface.FindAnimationRef( TPS8( bk_ftdn ) );

	if ( !pParentAnimRef )
		pParentAnimRef = rParentAnimInterface.FindAnimationRef( TPS8( idle_lp ) );

	const TPString8* pAnimName;

	if ( bOnOtis || pParentAnimRef )
		pAnimName = &strIdleAnim;
	else if ( rParentAnimInterface.FindAnimationRef( TPS8( walk_lp ) ) || rAnimInterface.FindAnimationRef( TPS8( bk_slow ) ) )
		pAnimName = &TPS8( onhead_walk );
	else
		pAnimName = &TPS8( onhead_movingfast );

	if ( !rAnimInterface.IsAnimPlaying( *pAnimName, TFALSE ) )
		rAnimInterface.PlayAnim( *pAnimName, -1.0f, TFALSE );
}
