#include "pch.h"
#include "ACameraManager.h"
#include "Tasks/ARootTask.h"
#include "ABoneAttachCameraHelper.h"
#include "ALerpCameraHelper.h"
#include "ASSeqCameraHelper.h"
#include "ABikePhysicsCameraHelper.h"
#include "AGolfCameraHelper.h"
#include "AVictoryCameraHelper.h"
#include "AFollowCameraHelper.h"
#include "AConversationCameraHelper.h"
#include "AGTAStyleCameraHelper.h"
#include "APoolCamera.h"
#include "ASpeechCameraHelper.h"
#include "ATrackingCameraHelper.h"
#include "AMultiCameraHelper.h"
#include "AProjectileFireCameraHelper.h"
#include "ACarChaseCamera.h"
#include "Render/ADebugOverlay.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0045b810, CreateObject 0045c2e0, CreateObjectInPlace 0045c340
TDEFINE_CLASS( ACameraManager );

// $Barnyard: FUNCTION 0045b960
ACameraManager::ACameraManager()
    : m_vLerpPosFrom( TVector4::VEC_ZERO )
    , m_vInitialLookDirection( TVector4::VEC_POSZ )
    , m_fLerpDuration( 0.0f )
    , m_fLerpTime( 0.0f )
    , m_fLerpFOVFrom( 0.0f )
    , m_bIsPosLerping( TFALSE )
{
	for ( TUINT i = 0; i < SPLITSCREEN_MAX_CAMERAS; i++ )
	{
		m_apCameras[ i ]                         = new ACamera();
		m_apCurrentSplitscreenCameraHelpers[ i ] = TNULL;
	}

	m_pCurrentCamera = m_apCameras[ 0 ];

	m_apCameraHelpers[ CAMERAHELPER_BIKE ]            = new ABikePhysicsCameraHelper();
	m_apCameraHelpers[ CAMERAHELPER_GOLF ]            = new AGolfCameraHelper();
	m_apCameraHelpers[ CAMERAHELPER_VICTORY ]         = new AVictoryCameraHelper();
	m_apCameraHelpers[ CAMERAHELPER_FOLLOW ]          = new AFollowCameraHelper();
	m_apCameraHelpers[ CAMERAHELPER_CONVERSATION ]    = new AConversationCameraHelper();
	m_apCameraHelpers[ CAMERAHELPER_SSEQ ]            = new ASSeqCameraHelper();
	m_apCameraHelpers[ CAMERAHELPER_BONEATTACH ]      = new ABoneAttachCameraHelper();
	m_apCameraHelpers[ CAMERAHELPER_GTA ]             = new AGTAStyleCameraHelper();
	m_apCameraHelpers[ CAMERAHELPER_LERP ]            = new ALerpCameraHelper();
	m_apCameraHelpers[ CAMERAHELPER_POOL ]            = new APoolCamera();
	m_apCameraHelpers[ CAMERAHELPER_SPEECH ]          = new ASpeechCameraHelper();
	m_apCameraHelpers[ CAMERAHELPER_TRACKING ]        = new ATrackingCameraHelper();
	m_apCameraHelpers[ CAMERAHELPER_MULTICAMERA ]     = new AMultiCameraHelper();
	m_apCameraHelpers[ CAMERAHELPER_PROJECTILE_FIRE ] = new AProjectileFireCameraHelper();

	for ( TUINT i = 0; i < SPLITSCREEN_MAX_CAMERAS; i++ )
	{
		m_apSplitscreenCameraHelpers[ SPLITSCREEN_CAMERAHELPER_PROJECTILE_1 + i ] = new AProjectileFireCameraHelper();
		m_apSplitscreenCameraHelpers[ SPLITSCREEN_CAMERAHELPER_GTA_1 + i ]        = new AGTAStyleCameraHelper();
		m_apSplitscreenCameraHelpers[ SPLITSCREEN_CAMERAHELPER_CAR_CHASE_1 + i ]  = new ACarChaseCamera();
	}

	m_pCurrentHelper        = TNULL;
	m_pPreviousCameraHelper = TNULL;

	for ( TINT i = 0; i < CAMERAHELPER_NUM_OF; i++ )
	{
		m_apCameraHelpers[ i ]->UpdateCurrentCamera();
		m_apCameraHelpers[ i ]->SetCamera( m_pCurrentCamera );
	}

	// Every player gets his own camera
	for ( TINT i = 0; i < SPLITSCREEN_CAMERAHELPER_NUM_OF / SPLITSCREEN_MAX_CAMERAS; i++ )
	{
		for ( TUINT k = 0; k < SPLITSCREEN_MAX_CAMERAS; k++ )
		{
			ACameraHelper* pHelper = m_apSplitscreenCameraHelpers[ i * SPLITSCREEN_MAX_CAMERAS + k ];
			pHelper->UpdateCurrentCamera();
			pHelper->SetCamera( m_apCameras[ k ] );
		}
	}

	TSTATICCAST( AProjectileFireCameraHelper, m_apCameraHelpers[ CAMERAHELPER_PROJECTILE_FIRE ] )->SetSpeed( 5.0f );
}

// $Barnyard: FUNCTION 0045be20
// $Barnyard: FUNCTION 0045c380
ACameraManager::~ACameraManager()
{
	for ( TUINT i = 0; i < SPLITSCREEN_MAX_CAMERAS; i++ )
	{
		if ( m_pCurrentCamera == m_apCameras[ i ] )
			m_pCurrentCamera = TNULL;

		if ( m_apCameras[ i ] )
			delete m_apCameras[ i ];
	}

	if ( m_pCurrentCamera )
		delete m_pCurrentCamera;

	for ( TINT i = 0; i < CAMERAHELPER_NUM_OF; i++ )
	{
		if ( m_apCameraHelpers[ i ] )
			delete m_apCameraHelpers[ i ];
	}

	for ( TINT i = 0; i < SPLITSCREEN_CAMERAHELPER_NUM_OF; i++ )
	{
		if ( m_apSplitscreenCameraHelpers[ i ] )
			delete m_apSplitscreenCameraHelpers[ i ];
	}
}

// $Barnyard: FUNCTION 0045c3a0
TBOOL ACameraManager::Reset()
{
	ResetHelperTargets();
	return TTRUE;
}

// $Barnyard: FUNCTION 0045bf10
TBOOL ACameraManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( ARootTask::GetSingleton()->IsPaused() )
		return TTRUE;

#ifdef ADEBUGOVERLAY
	// Development: the camera on the debug overlay
	if ( ADebugOverlay::IsPrinting() && GetCurrentCamera() )
	{
		const TVector4& rcPosition = GetCurrentCamera()->GetMatrix().GetTranslation();
		ADEBUGLINE( LINE_CAMERA, ADebugOverlay::INFO_COLOUR, "Camera Position(%f,%f,%f)", rcPosition.x, rcPosition.y, rcPosition.z );
	}
#endif

	if ( !m_bIsPosLerping )
	{
		// No smooth transition is happening, just update the helpers
		if ( m_pCurrentHelper )
			m_pCurrentHelper->OnUpdate( a_fDeltaTime );

		for ( TINT i = 0; i < SPLITSCREEN_MAX_CAMERAS; i++ )
		{
			if ( m_apCurrentSplitscreenCameraHelpers[ i ] )
				m_apCurrentSplitscreenCameraHelpers[ i ]->OnUpdate( a_fDeltaTime );
		}

		return TTRUE;
	}

	if ( m_fLerpTime >= m_fLerpDuration )
	{
		StopPositionLerp( a_fDeltaTime );
		return TTRUE;
	}

	if ( !m_pCurrentHelper )
	{
		m_bIsPosLerping = TFALSE;
		return TTRUE;
	}

	// Let the helper move its camera and blend from the stored state to it
	m_pCurrentHelper->OnUpdate( a_fDeltaTime );

	ACamera* pHelperCamera = m_pCurrentHelper->GetCamera();
	TFLOAT   fProgress     = m_fLerpTime / m_fLerpDuration;

	TVector4 vCameraPos;
	vCameraPos.Lerp4( m_vLerpPosFrom, pHelperCamera->GetMatrix().GetTranslation(), fProgress );

	TVector4 vOrientation = pHelperCamera->GetOrientation();
	TFLOAT   fFOV         = TMath::LERP( m_fLerpFOVFrom, m_pCurrentHelper->GetFOV(), fProgress );

	TVector4 vLookDirection;
	vLookDirection.Lerp4( m_vInitialLookDirection, vOrientation, fProgress );
	vLookDirection.Normalise();

	m_pCurrentCamera->SetFOV( fFOV );
	m_pCurrentCamera->GetMatrix().GetTranslation() = vCameraPos;
	m_pCurrentCamera->LookAtDirection( vLookDirection );

	m_fLerpTime += a_fDeltaTime;
	return TTRUE;
}

// $Barnyard: FUNCTION 0045c170
void ACameraManager::SetSmoothTransition( ACameraHelper* a_pHelper, TFLOAT a_fDuration )
{
	if ( !m_pCurrentCamera )
		SetCameraHelper( a_pHelper );

	if ( m_pCurrentHelper != a_pHelper )
	{
		// Remember the current state to blend from
		const TMatrix44& rcMatrix = m_pCurrentCamera->GetMatrix();

		m_vLerpPosFrom            = rcMatrix.GetTranslation();
		m_vInitialLookDirection   = rcMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
		m_vInitialLookDirection.w = 1.0f;
		m_fLerpFOVFrom            = m_pCurrentCamera->GetFOV();

		m_pPreviousCameraHelper = m_pCurrentHelper;
		m_pCurrentHelper        = a_pHelper;
		a_pHelper->OnAttach();

		m_bIsPosLerping = TTRUE;
		m_fLerpTime     = 0.0f;
		m_fLerpDuration = a_fDuration;

		OnUpdate( 1.0f / 30.0f );
	}
}

// $Barnyard: FUNCTION 0045c290
void ACameraManager::ResetHelperTargets()
{
	for ( TINT i = 0; i < CAMERAHELPER_NUM_OF; i++ )
	{
		m_apCameraHelpers[ i ]->ResetTarget();
	}

	for ( TINT i = 0; i < SPLITSCREEN_CAMERAHELPER_NUM_OF; i++ )
	{
		m_apSplitscreenCameraHelpers[ i ]->ResetTarget();
	}
}

// $Barnyard: FUNCTION 0045c0f0
void ACameraManager::SetCameraHelper( ACameraHelper* a_pHelper )
{
	// Detach split screen camera helpers
	for ( TINT i = 0; i < SPLITSCREEN_MAX_CAMERAS; i++ )
	{
		if ( m_apCurrentSplitscreenCameraHelpers[ i ] )
		{
			m_apCurrentSplitscreenCameraHelpers[ i ]->OnDetach();
			m_apCurrentSplitscreenCameraHelpers[ i ] = TNULL;
		}
	}

	// Detach currently attached camera helper
	if ( m_pCurrentHelper && m_pCurrentHelper != a_pHelper )
	{
		m_pCurrentHelper->OnDetach();
		m_pPreviousCameraHelper = m_pCurrentHelper;
	}

	m_pCurrentHelper = a_pHelper;

	if ( m_pCurrentHelper )
	{
		m_pCurrentHelper->OnCreate();
		m_pCurrentHelper->OnAttach();
	}

	m_pCurrentHelper->SetCamera( m_pCurrentCamera );
	m_bIsPosLerping = TFALSE;
}

// $Barnyard: FUNCTION 0045c240
void ACameraManager::DetachCameraHelpers()
{
	if ( m_pCurrentHelper )
		m_pCurrentHelper->OnDetach();

	m_pCurrentHelper = TNULL;

	for ( TINT i = 0; i < SPLITSCREEN_MAX_CAMERAS; i++ )
	{
		if ( m_apCurrentSplitscreenCameraHelpers[ i ] )
		{
			m_apCurrentSplitscreenCameraHelpers[ i ]->OnDetach();
			m_apCurrentSplitscreenCameraHelpers[ i ] = TNULL;
		}
	}
}

// $Barnyard: FUNCTION 0045b890
void ACameraManager::DetachSplitscreenCameraHelpers()
{
	for ( TINT i = 0; i < SPLITSCREEN_MAX_CAMERAS; i++ )
	{
		if ( m_apCurrentSplitscreenCameraHelpers[ i ] )
		{
			m_apCurrentSplitscreenCameraHelpers[ i ]->OnDetach();
			m_apCurrentSplitscreenCameraHelpers[ i ] = TNULL;
		}
	}
}

// $Barnyard: FUNCTION 0045c3b0
void ACameraManager::SetCameraHelper( CAMERAHELPER a_eCameraHelper )
{
	SetCameraHelper( m_apCameraHelpers[ a_eCameraHelper ] );
}

// $Barnyard: FUNCTION 0045c3d0
void ACameraManager::SetSplitscreenCameraHelper( TUINT a_uiGroup )
{
	SetCameraHelper( m_apSplitscreenCameraHelpers[ a_uiGroup * SPLITSCREEN_MAX_CAMERAS ] );
}

ACamera* ACameraManager::GetCamera( TUINT a_uiCameraIndex ) const
{
	TASSERT( a_uiCameraIndex < SPLITSCREEN_MAX_CAMERAS );
	return m_apCameras[ a_uiCameraIndex ];
}

// $Barnyard: FUNCTION 0045c0d0
ACameraHelper* ACameraManager::GetCameraHelper( CAMERAHELPER a_eCameraHelper ) const
{
	TASSERT( a_eCameraHelper < CAMERAHELPER_NUM_OF );
	return m_apCameraHelpers[ a_eCameraHelper ];
}

// $Barnyard: FUNCTION 0045b880
ACameraHelper* ACameraManager::GetCurrentCameraHelper() const
{
	return m_pCurrentHelper;
}

// $Barnyard: FUNCTION 0045b870
ACamera* ACameraManager::GetCurrentCamera() const
{
	return m_pCurrentCamera;
}

// $Barnyard: FUNCTION 0045b820
void ACameraManager::StopPositionLerp( TFLOAT a_flDeltaTime )
{
	if ( !m_bIsPosLerping )
		return;

	m_bIsPosLerping = TFALSE;
	m_fLerpDuration = 0.0f;

	if ( m_pCurrentHelper )
	{
		m_pCurrentHelper->SetCamera( m_pCurrentCamera );
		m_pCurrentHelper->OnUpdate( a_flDeltaTime );
	}
}
