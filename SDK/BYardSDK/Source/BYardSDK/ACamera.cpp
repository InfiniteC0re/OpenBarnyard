#include "pch.h"
#include "ACamera.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

const Toshi::TVector4 ACamera::sm_vWorldUp = { 0.0f, -1.0f, 0.0f, 1.0f };

TVector4 ACamera::GetOrientation() const
{
	TVector4 vOrientation = m_Matrix.AsBasisVector4( Toshi::BASISVECTOR_FORWARD );
	vOrientation.w        = 1.0f;

	return vOrientation;
}

void ACamera::LookAtPoint( const Toshi::TVector4& a_rPoint )
{
	TVector4 vDirection = a_rPoint - m_Matrix.GetTranslation3();
	vDirection.Normalise();

	LookAtDirection( vDirection );
}

void ACamera::LookAtDirection( const Toshi::TVector4& a_rDirection )
{
	TASSERT( a_rDirection.isNormalised() );
	m_Matrix.LookAtDirection( a_rDirection, sm_vWorldUp );
}

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

ACameraHelper* ACameraManager::GetCameraHelper( CAMERAHELPER a_eCameraHelper ) const
{
	TASSERT( a_eCameraHelper < CAMERAHELPER_NUM_OF );
	return m_apCameraHelpers[ a_eCameraHelper ];
}

ACameraHelper* ACameraManager::GetCurrentCameraHelper() const
{
	return m_pCurrentHelper;
}

// $Barnyard: FUNCTION 0045c0d0
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
