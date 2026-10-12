#include "pch.h"
#include "ASSeqAttachmentManager.h"
#include "ASSeqActorData.h"
#include "ASSeqController.h"
#include "ASSeq.h"
#include "AUnitActor.h"
#include "AUnitManager.h"
#include "Animation/AModelRepos.h"
#include "MiniGame/AMiniGameManager.h"
#include "Terrain/ATerrainInterface.h"

#include <Toshi/TSystem.h>
#include <Toshi/TScheduler.h>
#include <Toshi/TStringManager.h>
#include <Render/TSkeleton.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00629060, CreateObject 00629420, CreateObjectInPlace 00629480
TDEFINE_CLASS( ASSeqAttachmentManager );

// $Barnyard: FUNCTION 00629280
ASSeqAttachmentManager::ASSeqAttachmentManager()
    : m_pModelInstance( TNULL )
    , m_iNumAttachments( 0 )
    , m_pAnimation( TNULL )
    , m_vecPosition( TVector4::VEC_ZERO )
{
	for ( TINT i = 0; i < MAX_NUM_ATTACHMENTS; i++ )
	{
		m_aAttachments[ i ].pActor = TNULL;
		m_aAttachments[ i ].iBone  = -1;
	}
}

// $Barnyard: FUNCTION 00629340
// $Barnyard: FUNCTION 00629070
ASSeqAttachmentManager::~ASSeqAttachmentManager()
{
}

// $Barnyard: FUNCTION 00629620
TBOOL ASSeqAttachmentManager::OnSequenceEvent( TINT a_iEvent, const void* a_pEventData )
{
	const SequenceEvent* pEvent = TSTATICCAST( const SequenceEvent, a_pEventData );

	if ( a_iEvent == EVENT_ANIMATION )
	{
		PlayAnimation( pEvent );
		return TFALSE;
	}

	if ( a_iEvent != EVENT_COMMAND )
		return TTRUE;

	if ( TStringManager::String8Compare( pEvent->szCommand, "parentNode_Config" ) == 0 )
	{
		TString8 strModelPath( "data\\models\\" );
		strModelPath += pEvent->szArgument1;
		strModelPath.MakeLower();

		TPString8    strModel( strModelPath.GetString() );
		AModelRepos* pModelRepos = AModelRepos::GetSingleton();
		AModel*      pModel      = pModelRepos->GetModel( strModel );
		TBOOL        bCreate     = TTRUE;

		if ( !m_pModelInstance )
		{
			pModelRepos->LoadModel( strModel, TNULL );
		}
		else
		{
			m_iNumAttachments = 0;

			if ( pModel && m_pModelInstance->GetModel() != pModel )
			{
#ifdef BARNYARD_COMMUNITY_PATCH
				// Retail reads the model of the instance after destroying it
				AModel* pOldModel = m_pModelInstance->GetModel();
				pModelRepos->DestroyModelInstance( m_pModelInstance );
				pModelRepos->UnloadModel( pOldModel->GetName(), TTRUE );
#else
				pModelRepos->DestroyModelInstance( m_pModelInstance );
				pModelRepos->UnloadModel( m_pModelInstance->GetModel()->GetName(), TTRUE );
#endif
				pModelRepos->LoadModel( strModel, TNULL );
			}
			else
			{
				// The same model is already created
				bCreate = TFALSE;
			}
		}

		if ( bCreate )
			m_pModelInstance = pModelRepos->InstantiateModel( pModelRepos->GetModel( strModel ) );

		if ( !m_pSSeq->HasTransform() )
		{
			m_pModelInstance->GetTransform().SetEuler( TVector3( TMath::HALF_PI, 0.0f, 0.0f ) );
			m_pModelInstance->SetTranslation( m_vecPosition.AsVector3() );
		}
		else
		{
			// Place the model relative to the terrain locators
			TMatrix44 matSSeq = m_pSSeq->GetTransform();

			// NOTE: retail copies the matrix of the transform object directly
			TMatrix44 matWorld = TREINTERPRETCAST( const TMatrix44&, ATerrainInterface::GetSingleton()->GetLocatorList()->GetWorldTransform() );

			TMatrix44 matTransform;
			matTransform.Multiply( matWorld, matSSeq );
			matTransform.GetTranslation().Add( m_vecPosition );

			m_pModelInstance->SetTransform( matTransform );
		}

		return TTRUE;
	}

	if ( TStringManager::String8Compare( pEvent->szCommand, "attachToParentNode" ) != 0 )
		return TTRUE;

	TPString8 strActor( pEvent->szArgument1 );
	AUnit*    pActor = AUnitManager::GetSingleton()->FindActorBySSeqId( ASSeqActorData::FindActorIndex( strActor.GetString() ) );
	TSTATICCAST( AAnimatedUnit, pActor )->GetAnimatableObject()->DestroyAnimControllers();

	TString8     strBone( pEvent->szArgument2 );
	const TCHAR* szScript = ASSeqController::GetSingleton()->GetCurrentScriptName();
	TINT         iVariant = AMiniGameManager::GetSingleton()->GetMiniGameVariant();

	// The coyote variant of Chick Coop Defender uses the racoon sequences
	if ( TStringManager::String8Compare( szScript, "SSCCDef_ChickCoopDefender" ) == 0 && iVariant == 1 )
	{
		if ( TStringManager::String8Compare( strBone.GetString(), "CH_Actor_GenRac01" ) == 0 )
			strBone = "CH_Actor_GenCoy01";
		else if ( TStringManager::String8Compare( strBone.GetString(), "CH_Actor_GenRac02" ) == 0 )
			strBone = "CH_Actor_GenCoy02";
		else if ( TStringManager::String8Compare( strBone.GetString(), "CH_Actor_GenRac03" ) == 0 )
			strBone = "CH_Actor_GenCoy03";
	}

	strBone.MakeLower();

	TPString8            strBoneName( strBone.GetString() );
	TINT                 iBone           = m_pModelInstance->GetBoneIndex( strBoneName );
	TINT                 iNumAttachments = m_iNumAttachments;
	ASSeqActorInterface* pActorInterface = TSTATICCAST( AUnitActor, pActor );

	for ( TINT i = 0; i < m_iNumAttachments; i++ )
	{
		if ( m_aAttachments[ i ].pActor == pActorInterface )
		{
#ifdef BARNYARD_COMMUNITY_PATCH
			m_aAttachments[ i ].iBone = iBone;
#else
			// Retail updates the slot after the last attachment instead
			m_aAttachments[ iNumAttachments ].iBone = iBone;
#endif
			return TTRUE;
		}
	}

	m_aAttachments[ iNumAttachments ].pActor = pActorInterface;
	m_aAttachments[ iNumAttachments ].iBone  = iBone;
	m_iNumAttachments += 1;

	return TTRUE;
}

// $Barnyard: FUNCTION 006294c0
TBOOL ASSeqAttachmentManager::SetActive( TBOOL a_bActive, ASSeq* a_pSSeq, TINT a_iUnused )
{
	m_pSSeq = a_pSSeq;

	if ( a_bActive == TTRUE )
	{
		m_oAnimUpdateStartListener.Connect(
		    ARenderer::GetSingleton()->GetAnimationUpdateStartEmitter(),
		    this,
		    &AnimationUpdateStartListener,
		    0
		);

		m_eState  = STATE_IDLE;
		m_bActive = TTRUE;
		return TTRUE;
	}

	for ( TINT i = 0; i < MAX_NUM_ATTACHMENTS; i++ )
	{
		m_aAttachments[ i ].pActor = TNULL;
		m_aAttachments[ i ].iBone  = -1;
	}

	m_iNumAttachments = 0;
	m_oAnimUpdateStartListener.Disconnect();
	m_bActive = TFALSE;
	m_eState  = STATE_IDLE;

	return TTRUE;
}

// $Barnyard: FUNCTION 00629360
TBOOL ASSeqAttachmentManager::UpdateEvent( TINT a_iEvent, const void* a_pEventData, TFLOAT a_fDeltaTime )
{
	if ( !m_pModelInstance || m_eState != STATE_PLAYING )
		return TTRUE;

	return UpdateAnimation( a_pEventData, a_fDeltaTime );
}

// $Barnyard: FUNCTION 00629250
void ASSeqAttachmentManager::StopSequenceAnimation()
{
	if ( m_eState == STATE_PLAYING )
	{
		// NOTE: the actors stay attached, only the animation is forgotten
		m_pModelInstance->GetSkeletonInstance()->RemoveAllAnimations();
		m_eState     = STATE_IDLE;
		m_pAnimation = TNULL;
	}
}

// $Barnyard: FUNCTION 006291f0
void ASSeqAttachmentManager::RemoveAttachment( ASSeqActorInterface* a_pActor )
{
	TINT iIndex;

	for ( iIndex = 0; iIndex < m_iNumAttachments; iIndex++ )
	{
		if ( m_aAttachments[ iIndex ].pActor == a_pActor )
			break;
	}

	if ( iIndex >= m_iNumAttachments )
		return;

	// Shift the rest of the array (retail always moves up to the end of it)
	for ( TINT i = iIndex; i < MAX_NUM_ATTACHMENTS - 1; i++ )
		m_aAttachments[ i ] = m_aAttachments[ i + 1 ];

	m_iNumAttachments -= 1;
}

// $Barnyard: FUNCTION 006293b0
void ASSeqAttachmentManager::Reset()
{
	if ( m_pModelInstance )
	{
		m_pModelInstance->GetSkeletonInstance()->RemoveAllAnimations();
		m_pAnimation = TNULL;

		AModelRepos* pModelRepos = AModelRepos::GetSingleton();

#ifdef BARNYARD_COMMUNITY_PATCH
		// Retail reads the model of the instance after destroying it
		AModel* pModel = m_pModelInstance->GetModel();
		pModelRepos->DestroyModelInstance( m_pModelInstance );
		pModelRepos->UnloadModel( pModel->GetName(), TTRUE );
#else
		pModelRepos->DestroyModelInstance( m_pModelInstance );
		pModelRepos->UnloadModel( m_pModelInstance->GetModel()->GetName(), TTRUE );
#endif

		m_pModelInstance = TNULL;
	}

	for ( TINT i = 0; i < MAX_NUM_ATTACHMENTS; i++ )
	{
		m_aAttachments[ i ].pActor = TNULL;
		m_aAttachments[ i ].iBone  = -1;
	}

	m_iNumAttachments = 0;
}

// $Barnyard: FUNCTION 00629540
void ASSeqAttachmentManager::PlayAnimation( const SequenceEvent* a_pEvent )
{
	if ( !m_pModelInstance )
		return;

	TPString8          strAnimation( a_pEvent->szArgument1 );
	TSkeletonInstance* pSkeletonInstance = m_pModelInstance->GetSkeletonInstance();

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: once the previous animation has blended out (UpdateAnimation keeps the pointer then),
	// the skeleton has already removed it and retail removes it once more (TSkeleton asserts)
	if ( m_pAnimation && m_pAnimation->IsActive() )
		pSkeletonInstance->RemoveAnimation( m_pAnimation, 0.0f );
#else
	if ( m_pAnimation )
		pSkeletonInstance->RemoveAnimation( m_pAnimation, 0.0f );
#endif

	TINT iSequence = pSkeletonInstance->GetSkeleton()->GetSequenceID( strAnimation.GetString8().GetString(), 0 );
	m_pAnimation   = pSkeletonInstance->AddAnimation( TUINT16( iSequence ), 1.0f, 0.0f );
	m_pAnimation->SetSpeed( 1.0f );

	m_eState = STATE_PLAYING;
}

// $Barnyard: FUNCTION 006290d0
TBOOL ASSeqAttachmentManager::UpdateAnimation( const void* a_pEventData, TFLOAT a_fDeltaTime )
{
	if ( IsAnimationBlendingOutDone( m_pAnimation, a_fDeltaTime ) != TTRUE )
	{
		// NOTE: the rest of the event data layout is unknown
		TINT   iNumLoops = *TREINTERPRETCAST( const TINT*, TUINTPTR( a_pEventData ) + 0x14 );
		TFLOAT fDuration = m_pAnimation->GetSequencePtr()->GetDuration();

		if ( iNumLoops > 0 )
			fDuration *= iNumLoops;

		// Still playing
		if ( m_pAnimation->GetTotalTime() <= fDuration - TMath::TFLOAT_EPSILON )
			return TFALSE;

		m_pModelInstance->GetSkeletonInstance()->RemoveAnimation( m_pAnimation, 0.0f );
		m_pAnimation = TNULL;
	}

	m_eState = STATE_IDLE;
	return TTRUE;
}

// $Barnyard: FUNCTION 00629160
TBOOL ASSeqAttachmentManager::OnAnimationUpdateStart( ARenderer* a_pRenderer, TBOOL** a_ppData )
{
	if ( m_pModelInstance && m_pAnimation )
	{
		TSkeletonInstance* pSkeletonInstance = m_pModelInstance->GetSkeletonInstance();

		pSkeletonInstance->UpdateTime( g_oSystemManager.GetScheduler()->GetCurrentDeltaTime() );
		pSkeletonInstance->UpdateState( TTRUE ); // NOTE: same call as in ABoneAttachCameraHelper::AnimationStartEventHandler

		// Move the attached actors to their bones
		for ( TINT i = 0; i < m_iNumAttachments; i++ )
		{
			TMatrix44 matBone;
			m_pModelInstance->GetBoneTransform( m_aAttachments[ i ].iBone, matBone );
			m_aAttachments[ i ].pActor->SetTransform( matBone );
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00629390
TBOOL __stdcall ASSeqAttachmentManager::AnimationUpdateStartListener( ASSeqAttachmentManager* a_pCaller, ARenderer* a_pOwner, TBOOL** a_ppData )
{
	return a_pCaller->OnAnimationUpdateStart( a_pOwner, a_ppData );
}
