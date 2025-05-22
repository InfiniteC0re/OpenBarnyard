#include "pch.h"
#include "ACameraManager.h"
#include "Tasks/ARootTask.h"
#include "ABoneAttachCameraHelper.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( ACameraManager );

// $Barnyard: FUNCTION 0045b960
ACameraManager::ACameraManager()
    : m_vInitialLookDirection( ACamera::sm_vInitialLookDirection )
    , m_bIsPosLerping( TFALSE )
{
	for ( TUINT i = 0; i < SPLITSCREEN_MAX_CAMERAS; i++ )
	{
		m_apCameras[ i ]                  = new ACamera();
		m_apSplitscreenCameraHelpers[ i ] = TNULL;
	}

	for ( TINT i = 0; i < CAMERAHELPER_NUM_OF; i++ )
		m_apCameraHelpers[ i ] = TNULL;

	for ( TINT i = 0; i < SPLITSCREEN_CAMERAHELPER_NUM_OF; i++ )
		m_apSplitscreenCameraHelpers[ i ] = TNULL;

	for ( TINT i = 0; i < SPLITSCREEN_MAX_CAMERAS; i++ )
		m_apCurrentSplitscreenCameraHelpers[ i ] = TNULL;

	m_pCurrentCamera = GetCamera( 0 );

	TTODO( "Create helpers" );
	m_apCameraHelpers[ CAMERAHELPER_BONEATTACH ] = new ABoneAttachCameraHelper();
	// ...

	m_pCurrentHelper        = TNULL;
	m_pPreviousCameraHelper = TNULL;

	for ( TINT i = 0; i < CAMERAHELPER_NUM_OF; i++ )
	{
		if ( m_apCameraHelpers[ i ] )
			m_apCameraHelpers[ i ]->UpdateCurrentCamera();
	}

	for ( TINT i = 0; i < SPLITSCREEN_CAMERAHELPER_NUM_OF; i++ )
	{
		if ( m_apSplitscreenCameraHelpers[ i ] )
			m_apSplitscreenCameraHelpers[ i ]->UpdateCurrentCamera();
	}

	// ...
}

ACameraManager::~ACameraManager()
{
}

// $Barnyard: FUNCTION 0045bf10
TBOOL ACameraManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	TIMPLEMENT();

	if ( ARootTask::GetSingleton()->IsPaused() )
		return TTRUE;

	if ( !m_bIsPosLerping )
	// No smooth transition is happening, just update the helpers
	{
		if ( m_pCurrentHelper )
			m_pCurrentHelper->OnUpdate( a_fDeltaTime );

		for ( TINT i = 0; i < SPLITSCREEN_CAMERAHELPER_NUM_OF; i++ )
		{
			if ( m_apSplitscreenCameraHelpers[ i ] )
				m_apSplitscreenCameraHelpers[ i ]->OnUpdate( a_fDeltaTime );
		}

		return TTRUE;
	}

	TASSERT( !"Smooth camera transition is not implemented" );

	//static TFLOAT s_fTime = 0.0f;
	//s_fTime += a_fDeltaTime;

	//TVector4 vLookDirection = m_vInitialLookDirection;
	//vLookDirection.x        = -0.6f;
	//vLookDirection.z        = 1.0f;
	//vLookDirection.y        = 0.5f;
	//vLookDirection.Negate();
	//vLookDirection.Normalise();

	//auto& rvTranslation = m_pCurrentCamera->GetMatrix().GetTranslation();
	////rvTranslation.x     = -3.0f;
	////rvTranslation.y     = -18.0f + cos( s_fTime / 2.0f ) * 9.0f;
	////rvTranslation.z     = -16 + cos( s_fTime / 3.0f ) * 16.0f;
	//rvTranslation.x     = 32.78f;
	//rvTranslation.y     = -9.41f;
	//rvTranslation.z     = -31.17f;

	//m_pCurrentCamera->LookAtDirection( vLookDirection );

	return TTRUE;
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
