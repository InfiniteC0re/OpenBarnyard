#include "pch.h"
#include "AUnitActor.h"
#include "AUnitManager.h"
#include "ASSeqActorData.h"
#include "ASSeqController.h"
#include "Animation/AAnimatableObjectManager.h"

#include <Toshi/TSceneObject.h>
#include <Toshi/TStringManager.h>
#include <Render/TSkeleton.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00419580, CreateObject 00419610, CreateObjectInPlace 00419670
TDEFINE_CLASS( AUnitActor, 1, 1 );

// $Barnyard: FUNCTION 00419590
AUnitActor::AUnitActor()
    : m_pAnimation( TNULL )
    , m_bForceAlwaysUpdate( TFALSE )
    , m_bSkeletonActive( TFALSE )
{
}

// $Barnyard: FUNCTION 004196b0
// $Barnyard: FUNCTION 004195e0
// $Barnyard: FUNCTION 00419600
AUnitActor::~AUnitActor()
{
	// NOTE: 00419600 is the this-adjusting thunk of the AGoalCollisionUpdateInterface (+0x40) vtable
}

// $Barnyard: FUNCTION 004196d0
TBOOL AUnitActor::Create( AUnitTransform* a_pTransform )
{
	if ( !AAnimatedUnit::Create( a_pTransform ) )
		return TFALSE;

	m_eFlags |= FLAGS_AI_ENABLED;
	m_eFlags |= FLAGS_THINK_ENABLED;

	GetAnimatableObject()->GetModelInstance()->GetSceneObject()->DisableDistanceCulling();

	return TTRUE;
}

// NOTE: same code as ACharacterUnit::OnDestroy (00507930), folded in retail
void AUnitActor::OnDestroy()
{
	AAnimatedUnit::OnDestroy();
}

// NOTE: same code as AUnitPlayer::UpdateAI (00414890), folded in retail
void AUnitActor::UpdateAI( TFLOAT a_fDeltaTime )
{
	AVisibleUnit::UpdateAI( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00419740
TBOOL AUnitActor::Think( TFLOAT a_fDeltaTime )
{
	if ( m_bForceAlwaysUpdate )
	{
		AToshiAnimationInterface& rAnimInterface = GetAnimatableObject()->GetToshiAnimInterface();
		rAnimInterface.SetFlags( rAnimInterface.GetFlags() | AToshiAnimationInterface::FLAGS_ALWAYS_UPDATE );

		TSkeletonInstance* pSkeletonInstance = GetAnimatableObject()->GetModelInstance()->GetSkeletonInstance();
		pSkeletonInstance->SetFlags( pSkeletonInstance->GetFlags() | TSkeletonInstance::FLAGS_UPDATE_STATE );
	}

	GetAnimatableObject()->GetModelInstance()->GetSceneObject()->DisableDistanceCulling();

	return AVisibleUnit::Think( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00419810
void AUnitActor::UpdateActor( TFLOAT a_fDeltaTime )
{
	// NOTE: when the debug flag at 0x0077e884 is set, retail walks the base animations of
	// the skeleton instance without doing anything with them. Nothing else happens here.
}

// $Barnyard: FUNCTION 00419e20
TBOOL AUnitActor::OnSequenceEvent( TINT a_iEvent, const void* a_pEventData )
{
	if ( !IsIdle() )
		return TFALSE;

	const SequenceEvent* pEvent = TSTATICCAST( const SequenceEvent, a_pEventData );

	if ( a_iEvent == EVENT_ANIMATION )
	{
		PlaySequenceAnimation( pEvent );
		return TTRUE;
	}

	if ( a_iEvent != EVENT_COMMAND )
		return TTRUE;

	if ( TStringManager::String8Compare( pEvent->szCommand, "forceAllwaysUpdate" ) == 0 )
	{
		m_bForceAlwaysUpdate = TTRUE;
	}
	else if ( TStringManager::String8Compare( pEvent->szCommand, "detachFromParentNode" ) == 0 )
	{
		ASSeqController::GetSingleton()->GetAttachmentManager().RemoveAttachment( this );
	}
	else if ( TStringManager::String8Compare( pEvent->szCommand, "Hide" ) == 0 )
	{
		GetAnimatableObject()->SetSkeletonUpdating( TFALSE, TTRUE );
	}
	else if ( TStringManager::String8Compare( pEvent->szCommand, "show" ) == 0 )
	{
		GetAnimatableObject()->SetSkeletonUpdating( TTRUE, TTRUE );
	}
	else if ( TStringManager::String8Compare( pEvent->szCommand, "attachToParentNode" ) == 0 )
	{
		// NOTE: retail lowercases the event strings in place
		TStringManager::String8ToLowerCase( const_cast<TCHAR*>( pEvent->szArgument1 ) );
		TStringManager::String8ToLowerCase( const_cast<TCHAR*>( pEvent->szArgument2 ) );

		TPString8 strUnit( pEvent->szArgument1 );
		TPString8 strParentBone( pEvent->szArgument2 );

		AUnit* pActor = AUnitManager::GetSingleton()->FindActorBySSeqId( ASSeqActorData::FindActorIndex( pEvent->szArgument1 ) );

		if ( pActor )
		{
			AAnimatableObject* pActorAnimObj = TSTATICCAST( AAnimatedUnit, pActor )->GetAnimatableObject();

			if ( pActorAnimObj->GetParentObject() )
				AAnimatableObjectManager::GetSingleton()->DetachObject( pActorAnimObj, pActorAnimObj->GetParentObject() );

			pActorAnimObj->DestroyAnimControllers();
			AAnimatableObjectManager::GetSingleton()->AttachObject( pActorAnimObj, GetAnimatableObject(), TPString8(), strParentBone, TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
		}
		else if ( !strUnit.IsEmpty() )
		{
			AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();
			AAnimatableObject*        pAnimObj        = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( strUnit ), this, 0 );

			pAnimObj->SetSkeletonUpdating( TTRUE, TTRUE );
			pAnimObjManager->AttachObject( pAnimObj, GetAnimatableObject(), TPString8(), strParentBone, TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 004198c0
TBOOL AUnitActor::SetActive( TBOOL a_bActive, ASSeq* a_pSSeq, TINT a_iUnused )
{
	if ( a_bActive == TTRUE )
	{
		m_pSSeq   = a_pSSeq;
		m_eState  = STATE_IDLE;
		m_bActive = TTRUE;
		GetAnimatableObject()->SetSkeletonUpdating( TTRUE, TTRUE );
		m_bSkeletonActive = TTRUE;
	}
	else
	{
		m_eState  = STATE_IDLE;
		m_bActive = TFALSE;
		GetAnimatableObject()->GetModelInstance()->GetSkeletonInstance()->RemoveAllAnimations();

		GetAnimatableObject()->SetAnimControllersDisabled( TFALSE );

		m_bSkeletonActive = TFALSE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0041a120
TBOOL AUnitActor::UpdateEvent( TINT a_iEvent, const void* a_pEventData, TFLOAT a_fDeltaTime )
{
	if ( m_eState != STATE_PLAYING )
		return TTRUE;

	return UpdateSequenceAnimation( a_pEventData, a_fDeltaTime );
}

// $Barnyard: FUNCTION 00419d10
void AUnitActor::SetTransform( const TMatrix44& a_rcTransform )
{
	GetAnimatableObject()->GetModelInstance()->SetTransform( a_rcTransform );
}

// $Barnyard: FUNCTION 00419d80
void AUnitActor::StopSequenceAnimation()
{
	if ( m_eState == STATE_PLAYING )
	{
		AToshiAnimationInterface& rAnimInterface = GetAnimatableObject()->GetToshiAnimInterface();
		rAnimInterface.SetFlags( rAnimInterface.GetFlags() & ~AToshiAnimationInterface::FLAGS_ALWAYS_UPDATE );

		GetAnimatableObject()->GetModelInstance()->GetSkeletonInstance()->RemoveAllAnimations();

		m_eState     = STATE_IDLE;
		m_pAnimation = TNULL;
	}
}

// $Barnyard: FUNCTION 00419b10
void AUnitActor::PlaySequenceAnimation( const SequenceEvent* a_pEvent )
{
	AToshiAnimationInterface& rAnimInterface = GetAnimatableObject()->GetToshiAnimInterface();

	if ( m_bSkeletonActive )
	{
		GetAnimatableObject()->SetAnimControllersDisabled( TTRUE );
		GetAnimatableObject()->GetModelInstance()->GetSkeletonInstance()->RemoveAllAnimations();
		m_bSkeletonActive = TFALSE;
	}

	if ( m_pAnimation )
	{
		GetAnimatableObject()->GetModelInstance()->GetSkeletonInstance()->RemoveAllAnimations();
		rAnimInterface.SetFlags( rAnimInterface.GetFlags() & ~AToshiAnimationInterface::FLAGS_ALWAYS_UPDATE );
		m_pAnimation = TNULL;
	}

	TPString8          strAnimation( a_pEvent->szArgument1 );
	TSkeletonInstance* pSkeletonInstance = GetAnimatableObject()->GetModelInstance()->GetSkeletonInstance();

	TINT iSequence = pSkeletonInstance->GetSkeleton()->GetSequenceID( strAnimation.GetString8().GetString(), 0 );
	m_pAnimation   = pSkeletonInstance->AddAnimation( TUINT16( iSequence ), 1.0f, 0.0f );
	m_pAnimation->SetSpeed( 1.0f );

	rAnimInterface.SetFlags( rAnimInterface.GetFlags() | AToshiAnimationInterface::FLAGS_ALWAYS_UPDATE );

	m_eState = STATE_PLAYING;
}

// $Barnyard: FUNCTION 00419990
TBOOL AUnitActor::UpdateSequenceAnimation( const void* a_pEventData, TFLOAT a_fDeltaTime )
{
	TBOOL bFinished = ( m_pSSeq == TNULL );

	// NOTE: the rest of the event data layout is unknown
	TINT iNumLoops = *TREINTERPRETCAST( const TINT*, TUINTPTR( a_pEventData ) + 0x14 );

	if ( m_pAnimation && IsAnimationBlendingOutDone( m_pAnimation, a_fDeltaTime ) == TTRUE )
	{
		bFinished = TTRUE;
	}
	else if ( iNumLoops < 1 )
	{
		bFinished = TTRUE;
	}
	else if ( m_pAnimation->GetTotalTime() <= m_pAnimation->GetSequencePtr()->GetDuration() * iNumLoops - TMath::TFLOAT_EPSILON )
	{
		// Still playing
		if ( !bFinished )
			return bFinished;
	}
	else
	{
		GetAnimatableObject()->GetModelInstance()->GetSkeletonInstance()->RemoveAllAnimations();

		AToshiAnimationInterface& rAnimInterface = GetAnimatableObject()->GetToshiAnimInterface();
		rAnimInterface.SetFlags( rAnimInterface.GetFlags() & ~AToshiAnimationInterface::FLAGS_ALWAYS_UPDATE );

		m_pAnimation = TNULL;
		bFinished    = TTRUE;
	}

	if ( iNumLoops > 0 )
	{
		TSkeletonInstance* pSkeletonInstance = GetAnimatableObject()->GetModelInstance()->GetSkeletonInstance();
		m_eState                             = STATE_IDLE;
		pSkeletonInstance->RemoveAllAnimations();
	}

	return bFinished;
}
