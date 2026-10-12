#include "pch.h"
#include "ASSeqCameraHelper.h"
#include "Animation/AModelRepos.h"
#include "Render/ARenderer.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/ASSeq.h"

#include <Toshi/TSystem.h>
#include <Toshi/TScheduler.h>
#include <Render/TSkeleton.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00467e30
TDEFINE_CLASS_NORUNTIME( ASSeqCameraHelper );

// $Barnyard: FUNCTION 004684b0
ASSeqCameraHelper::ASSeqCameraHelper()
    : m_pModelInstance( TNULL )
    , m_pSkeletonInstance( TNULL )
    , m_pAnimation( TNULL )
    , m_vecPosition( TVector4::VEC_ZERO )
    , m_vecTarget( TVector4::VEC_ZERO )
    , m_iCameraBone( -1 )
    , m_iTargetBone( -1 )
    , m_iUnk( 0 )
    , m_fSavedFOV( 0.0f )
    , m_fFOV( 0.0f )
    , m_fTargetFOV( -1.0f )
    , m_fFOVSpeed( 0.0f )
    , m_fSavedNear( 0.0f )
    , m_bUseTarget( TFALSE )
{
}

// $Barnyard: FUNCTION 00468590
// $Barnyard: FUNCTION 004682a0
ASSeqCameraHelper::~ASSeqCameraHelper()
{
	if ( m_pModelInstance )
		AModelRepos::GetSingleton()->DestroyModelInstance( m_pModelInstance );
}

// $Barnyard: FUNCTION 00467e40
TBOOL ASSeqCameraHelper::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_bActive == TTRUE && m_fTargetFOV != -1.0f )
	{
		// Animate the FOV ("anim_FOV" command)
		m_fFOV += a_fDeltaTime * m_fFOVSpeed;

		if ( m_fFOVSpeed <= 0.0f )
		{
			if ( m_fFOV < m_fTargetFOV )
			{
				m_fFOV       = m_fTargetFOV;
				m_fTargetFOV = -1.0f;
			}
		}
		else if ( m_fFOV > m_fTargetFOV )
		{
			m_fFOV       = m_fTargetFOV;
			m_fTargetFOV = -1.0f;
		}

		m_pCamera->SetFOV( m_fFOV );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00467ea0
TBOOL ASSeqCameraHelper::UpdateAnimation( const void* a_pEventData, TFLOAT a_fDeltaTime )
{
	if ( !m_pModelInstance )
		return TFALSE;

	if ( m_iCameraBone >= 0 )
	{
		TMatrix44 matBone;
		m_pModelInstance->GetBoneTransform( m_iCameraBone, matBone );

		// The camera looks along the bone
		TMatrix44 matRotation = TMatrix44::IDENTITY;
		matRotation.RotateX( TMath::PI );

		TMatrix44 matBoneCopy = matBone;
		matBone.Multiply( matBoneCopy, matRotation );

		if ( m_bUseTarget )
			m_vecPosition = matBone.GetTranslation();
		else
			m_pCamera->SetMatrix( matBone );
	}

	if ( m_bUseTarget == TTRUE )
	{
		TMatrix44 matBone;
		m_pModelInstance->GetBoneTransform( m_iTargetBone, matBone );
		m_vecTarget = matBone.GetTranslation();

		LookAt( m_vecPosition, m_vecTarget );
	}

	if ( IsAnimationBlendingOutDone( m_pAnimation, a_fDeltaTime ) != TTRUE )
	{
		// NOTE: the rest of the event data layout is unknown
		TINT   iNumLoops = *TREINTERPRETCAST( const TINT*, TUINTPTR( a_pEventData ) + 0x14 );
		TFLOAT fDuration = m_pAnimation->GetSequencePtr()->GetDuration();

		if ( iNumLoops > 0 )
			fDuration *= iNumLoops;

		// Still playing
		if ( m_pAnimation->GetTotalTime() <= fDuration - 0.1f )
			return TFALSE;

		m_pSkeletonInstance->RemoveAnimation( m_pAnimation, 0.0f );
		m_pAnimation = TNULL;
	}

	m_eState = STATE_IDLE;
	return TTRUE;
}

// $Barnyard: FUNCTION 00468020
void ASSeqCameraHelper::SetOffset( const void* a_pEventData )
{
	if ( m_pModelInstance )
	{
		// NOTE: the rest of the event data layout is unknown
		const TFLOAT* pOffset = TREINTERPRETCAST( const TFLOAT*, TUINTPTR( a_pEventData ) + 0xc );

		TVector4 vecPosition = m_pSSeq->GetPosition();
		vecPosition.x += pOffset[ 0 ];
		vecPosition.y += pOffset[ 1 ];
		vecPosition.z += pOffset[ 2 ];
		vecPosition.Add( m_pSSeq->GetTransform().GetTranslation() );

		m_pModelInstance->SetTranslation( vecPosition.AsVector3() );
	}
}

// $Barnyard: FUNCTION 004680d0
void ASSeqCameraHelper::StopSequenceAnimation()
{
	if ( m_eState == STATE_PLAYING )
	{
		m_pSkeletonInstance->RemoveAllAnimations();
		m_eState     = STATE_IDLE;
		m_pAnimation = TNULL;
	}
}

// $Barnyard: FUNCTION 004680f0
TBOOL ASSeqCameraHelper::UpdateEvent( TINT a_iEvent, const void* a_pEventData, TFLOAT a_fDeltaTime )
{
	if ( !m_pModelInstance )
		return TFALSE;

	if ( m_eState == STATE_PLAYING )
		return UpdateAnimation( a_pEventData, a_fDeltaTime );

	return TTRUE;
}

// $Barnyard: FUNCTION 00468130
TBOOL __stdcall ASSeqCameraHelper::AnimationUpdateStartListener( ASSeqCameraHelper* a_pCaller, ARenderer* a_pOwner, TBOOL** a_ppData )
{
	if ( !a_pCaller->m_pModelInstance )
		return TFALSE;

	if ( a_pCaller->m_pAnimation )
	{
		a_pCaller->m_pSkeletonInstance->UpdateTime( g_oSystemManager.GetScheduler()->GetCurrentDeltaTime() );
		a_pCaller->m_pSkeletonInstance->UpdateState( TTRUE ); // NOTE: same call as in ABoneAttachCameraHelper::AnimationStartEventHandler
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00468170
void ASSeqCameraHelper::OnDetach()
{
	m_oAnimUpdateStartListener.Disconnect();

	if ( m_pModelInstance )
	{
		m_pSkeletonInstance->RemoveAllAnimations();

		AModelRepos::GetSingleton()->DestroyModelInstance( m_pModelInstance );
		AModelRepos::GetSingleton()->UnloadModel( m_strModelName, TTRUE );

		m_pAnimation        = TNULL;
		m_pModelInstance    = TNULL;
		m_pSkeletonInstance = TNULL;
		m_iCameraBone       = -1;
	}

	ms_szTerrain = TNULL;
	m_fFOV       = 0.0f;
	m_fFOVSpeed  = 0.0f;
	m_bUseTarget = TFALSE;
	m_fTargetFOV = -1.0f;
}

// $Barnyard: FUNCTION 004681e0
TINT ASSeqCameraHelper::GetVISGroup()
{
	if ( ms_szTerrain )
		return ATerrainInterface::GetSingleton()->GetSectionIndex( ms_szTerrain );

	return -1;
}

// $Barnyard: FUNCTION 00468200
TBOOL ASSeqCameraHelper::SetActive( TBOOL a_bActive, ASSeq* a_pSSeq, TINT a_iUnused )
{
	if ( !m_pModelInstance )
		return TFALSE;

	if ( a_bActive == TTRUE )
	{
		if ( !m_bActive )
		{
			m_pSSeq      = a_pSSeq;
			m_eState     = STATE_IDLE;
			m_bActive    = TTRUE;
			m_fSavedFOV  = m_pCamera->GetFOV();
			m_fSavedNear = ARenderer::GetSingleton()->SetNear( 0.5f );

			ATerrainInterface::GetSingleton()->SetGetCurrentVISGroupCallback( GetVISGroup );
			return TTRUE;
		}
	}
	else if ( m_bActive == TTRUE )
	{
		m_eState  = STATE_IDLE;
		m_bActive = TFALSE;
		m_pCamera->SetFOV( m_fSavedFOV );
		ARenderer::GetSingleton()->SetNear( m_fSavedNear );

		ATerrainInterface::GetSingleton()->SetGetCurrentVISGroupCallback( ATerrainInterface::GetSectionPlayerIsIn );
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00468340
void ASSeqCameraHelper::PlayAnimation( const SequenceEvent* a_pEvent )
{
	if ( !m_pModelInstance )
		return;

	TPString8 strAnimation( a_pEvent->szArgument1 );
	TINT      iSequence = m_pSkeletonInstance->GetSkeleton()->GetSequenceID( strAnimation.GetString8().GetString(), 0 );

	m_pSkeletonInstance->RemoveAllAnimations();
	m_pAnimation = m_pSkeletonInstance->AddAnimation( TUINT16( iSequence ), 1.0f, 0.0f );

	m_pSkeletonInstance->UpdateTime( g_oSystemManager.GetScheduler()->GetCurrentDeltaTime() );
	m_pSkeletonInstance->UpdateState( TTRUE ); // NOTE: same call as in ABoneAttachCameraHelper::AnimationStartEventHandler

	if ( m_iCameraBone >= 0 )
	{
		TMatrix44 matBone;
		m_pModelInstance->GetBoneTransform( m_iCameraBone, matBone );

		TMatrix44 matRotation = TMatrix44::IDENTITY;
		matRotation.RotateX( TMath::PI );

		TMatrix44 matBoneCopy = matBone;
		matBone.Multiply( matBoneCopy, matRotation );

		m_pCamera->SetMatrix( matBone );
	}

	m_pAnimation->SetSpeed( 1.0f );
	m_eState = STATE_PLAYING;
}

// $Barnyard: FUNCTION 004685b0
TBOOL ASSeqCameraHelper::OnSequenceEvent( TINT a_iEvent, const void* a_pEventData )
{
	if ( !IsIdle() )
		return TFALSE;

	const SequenceEvent* pEvent = TSTATICCAST( const SequenceEvent, a_pEventData );

	if ( a_iEvent == 0 )
	{
		SetOffset( a_pEventData );
	}
	else if ( a_iEvent == EVENT_ANIMATION )
	{
		PlayAnimation( pEvent );
	}
	else if ( a_iEvent == EVENT_COMMAND )
	{
		if ( TStringManager::String8Compare( pEvent->szCommand, "bindCamera" ) == 0 )
		{
			TString8 strCameraBone( pEvent->szArgument1 );
			strCameraBone.MakeLower();

			TString8 strTargetBone( pEvent->szArgument2 );
			strTargetBone.MakeLower();

			m_iCameraBone = m_pSkeletonInstance->GetSkeleton()->GetBoneID( strCameraBone.GetString(), strCameraBone.Length() );

			if ( m_bUseTarget == TTRUE )
				m_iTargetBone = m_pSkeletonInstance->GetSkeleton()->GetBoneID( strTargetBone.GetString(), strTargetBone.Length() );
		}
		else if ( TStringManager::String8Compare( pEvent->szCommand, "Set_FOV" ) == 0 )
		{
			m_pCamera->SetFOV( TFLOAT( atof( pEvent->szArgument1 ) * 0.017453292 ) );
		}
		else if ( TStringManager::String8Compare( pEvent->szCommand, "anim_FOV" ) == 0 )
		{
			// NOTE: "anim_FOV" uses the third argument, at +0x18
			const TCHAR* szDuration = *TREINTERPRETCAST( const TCHAR* const*, TUINTPTR( a_pEventData ) + 0x18 );

			TFLOAT fFrom     = TFLOAT( atof( pEvent->szArgument1 ) );
			TFLOAT fTo       = TFLOAT( atof( pEvent->szArgument2 ) );
			TFLOAT fDuration = TFLOAT( atof( szDuration ) );
			TFLOAT fFOV      = TFLOAT( atof( pEvent->szArgument1 ) * 0.017453292 );

			m_pCamera->SetFOV( fFOV );
			m_fFOV       = fFOV;
			m_fTargetFOV = fTo * 0.017453292f;
			m_fFOVSpeed  = ( fTo - fFrom ) / fDuration * 0.017453292f;
		}
		else if ( TStringManager::String8Compare( pEvent->szCommand, "SetTerrain" ) == 0 )
		{
			ms_szTerrain = pEvent->szArgument1;
		}
		else if ( TStringManager::String8Compare( pEvent->szCommand, "SetNearClip" ) == 0 )
		{
			ARenderer::GetSingleton()->SetNear( TFLOAT( atof( pEvent->szArgument1 ) ) );
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00468870
void ASSeqCameraHelper::Setup( const TCHAR* a_szModelName, ASSeq* a_pSSeq, const TVector4& a_rcPosition )
{
	m_strModelName = TPString8( a_szModelName );

	AModelRepos* pModelRepos = AModelRepos::GetSingleton();

	if ( m_pModelInstance )
	{
		m_pSkeletonInstance->RemoveAllAnimations();
		m_pAnimation = TNULL;

		if ( m_pModelInstance->GetModel() != pModelRepos->GetModel( m_strModelName ) )
		{
			pModelRepos->DestroyModelInstance( m_pModelInstance );
			pModelRepos->LoadModel( m_strModelName, TNULL );

			m_pModelInstance    = pModelRepos->InstantiateModel( pModelRepos->GetModel( m_strModelName ) );
			m_pSkeletonInstance = m_pModelInstance->GetSkeletonInstance();
			m_pModelInstance->SetTranslation( a_rcPosition.AsVector3() );
		}
	}
	else
	{
		pModelRepos->LoadModel( m_strModelName, TNULL );

		m_pModelInstance = pModelRepos->InstantiateModel( pModelRepos->GetModel( m_strModelName ) );
		m_pModelInstance->SetTranslation( a_rcPosition.AsVector3() );
		m_pSkeletonInstance = m_pModelInstance->GetSkeletonInstance();
	}

	if ( a_pSSeq->HasTransform() )
	{
		// Place the model relative to the terrain locators
		TMatrix44 matSSeq = a_pSSeq->GetTransform();

		// NOTE: retail copies the matrix of the transform object directly
		TMatrix44 matWorld = TREINTERPRETCAST( const TMatrix44&, ATerrainInterface::GetSingleton()->GetLocatorList()->GetWorldTransform() );

		TMatrix44 matTransform;
		matTransform.Multiply( matWorld, matSSeq );

		// NOTE: retail adds a_rcPosition to the translation of matWorld after the multiplication, so it has no effect

		m_pModelInstance->SetTransform( matTransform );
	}
	else
	{
		m_pModelInstance->GetTransform().SetEuler( TVector3( TMath::HALF_PI, 0.0f, 0.0f ) );
	}

	m_oAnimUpdateStartListener.Connect(
	    ARenderer::GetSingleton()->GetAnimationUpdateStartEmitter(),
	    this,
	    &AnimationUpdateStartListener,
	    0
	);
}
