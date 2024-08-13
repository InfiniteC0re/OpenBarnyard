#include "pch.h"
#include "ACameraManager.h"
#include "Tasks/ARootTask.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( ACameraManager );

ACameraManager::ACameraManager() :
	m_vLookDirection( ACamera::sm_vInitialLookDirection )
{
	TTODO( "Create helpers" );

	for ( TUINT i = 0; i < MAX_CAMERAS; i++ )
	{
		m_apCameras[ i ]   = new ACamera();
		m_apSomeArray[ i ] = TNULL;
	}

	m_pCurrentCamera = GetCamera( 0 );
}

TBOOL ACameraManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	TIMPLEMENT();

	if ( ARootTask::GetSingleton()->IsPaused() )
		return TTRUE;

	static TFLOAT s_fTime = 0.0f;
	s_fTime += a_fDeltaTime;

	TVector4 vLookDirection = m_vLookDirection;
	//vLookDirection.x = 0.0f;
	//vLookDirection.y = 1.0f;
	//vLookDirection.z = 0.0f;
	vLookDirection.Normalise();

	auto& rvTranslation = m_pCurrentCamera->GetMatrix().GetTranslation();
	rvTranslation.x		= -3.0f;
	rvTranslation.y		= -18.0f + cos( s_fTime / 2.0f ) * 9.0f;
	rvTranslation.z		= -16 + cos( s_fTime / 3.0f ) * 16.0f;

	m_pCurrentCamera->LookAtDirection( vLookDirection );

	return TTRUE;
}

ACamera* ACameraManager::GetCamera( TUINT a_uiCameraIndex ) const
{
	TASSERT( a_uiCameraIndex < MAX_CAMERAS );
	return m_apCameras[ a_uiCameraIndex ];
}

ACamera* ACameraManager::GetCurrentCamera() const
{
	return m_pCurrentCamera;
}
