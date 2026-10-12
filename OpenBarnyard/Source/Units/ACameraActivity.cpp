#include "pch.h"
#include "ACameraActivity.h"
#include "Cameras/ACameraManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b42d0, CreateObject 005b4480
// NOTE: GetClass 005b42d0, CreateObject 005b4480, CreateObjectInPlace 005b44f0
TDEFINE_CLASS( ACameraActivity );

TPSTRING8_DECLARE( nohelpercameras );
TPSTRING8_DECLARE( aconversationcamerahelper );
TPSTRING8_DECLARE( afollowcamerahelper );
TPSTRING8_DECLARE( aprojectilefirecamerahelper );
TPSTRING8_DECLARE( simcamera );

ACameraActivity::ACameraActivity()
{
}

// $Barnyard: FUNCTION 005b4360
// $Barnyard: FUNCTION 005b4560
ACameraActivity::~ACameraActivity()
{
}

// $Barnyard: FUNCTION 005b43d0
TBOOL ACameraActivity::Create( const PBProperties* a_pProperties )
{
	// NOTE: retail doesn't check if "type" is set
	m_strType = a_pProperties->GetProperty( "type" )->GetTPString8();

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b42e0
AInteractionActivity::RESULT ACameraActivity::Update( AInteractionSequence* a_pSequence )
{
	ACameraManager* pCameraManager = ACameraManager::GetSingleton();

	if ( m_strType == TPS8( nohelpercameras ) )
	{
		pCameraManager->DetachCameraHelpers();
		return RESULT_DONE;
	}

	// NOTE: ACameraManager::SetCameraHelper and ACameraManager::SetSplitscreenCameraHelper are inlined
	if ( m_strType == TPS8( aconversationcamerahelper ) )
	{
		pCameraManager->SetCameraHelper( pCameraManager->m_apCameraHelpers[ ACameraManager::CAMERAHELPER_CONVERSATION ] );
	}
	else if ( m_strType == TPS8( afollowcamerahelper ) )
	{
		pCameraManager->SetCameraHelper( pCameraManager->m_apCameraHelpers[ ACameraManager::CAMERAHELPER_FOLLOW ] );
	}
	else if ( m_strType != TPS8( aprojectilefirecamerahelper ) && m_strType == TPS8( simcamera ) )
	{
		// The GTA style camera helper of the first player
		pCameraManager->SetCameraHelper( pCameraManager->GetSplitscreenCameraHelper( 1 ) );
	}

	return RESULT_DONE;
}
