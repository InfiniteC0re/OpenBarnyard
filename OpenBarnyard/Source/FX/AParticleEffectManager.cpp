#include "pch.h"
#include "AParticleEffectManager.h"
#include "Animation/AModel.h"
#include "Assets/AMaterialLibraryManager.h"
#include "Cameras/ACameraManager.h"
#include "Render/AShadowManager.h"
#include "Render/ARenderer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00650b10, CreateObject 00651280, CreateObjectInPlace 006512e0
TDEFINE_CLASS( AParticleEffectManager );

// $Barnyard: FUNCTION 00650c30
AParticleEffectHandle::AParticleEffectHandle()
    : m_pInstance( TNULL )
    , m_fnOnFinished( TNULL )
    , m_pOnFinishedUserData( TNULL )
    , m_pAttachModel( TNULL )
    , m_iAttachBone( -1 )
    , m_eFlags( 0 )
{
}

// $Barnyard: FUNCTION 00650c60
TBOOL AParticleEffectHandle::Update( TFLOAT a_fDeltaTime )
{
	TMatrix44& rTransform = m_pInstance->GetTransform();

	if ( m_pAttachModel )
	{
		TMatrix44 mBone;
		m_pAttachModel->GetBoneTransform( m_iAttachBone, mBone );

		if ( HASANYFLAG( m_eFlags, FLAGS_FOLLOW_POSITION ) )
			rTransform.GetTranslation() = mBone.GetTranslation();

		if ( HASANYFLAG( m_eFlags, FLAGS_FOLLOW_ROTATION ) )
		{
			rTransform.AsBasisVector4( 0 ) = mBone.AsBasisVector4( 0 );
			rTransform.AsBasisVector4( 1 ) = mBone.AsBasisVector4( 1 );
			rTransform.AsBasisVector4( 2 ) = mBone.AsBasisVector4( 2 );
			rTransform.RotateX( -TMath::HALF_PI );
		}

		if ( HASANYFLAG( m_eFlags, FLAGS_FACE_CAMERA ) )
		{
			// Move the effect towards the camera
			ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

			TVector4 vecDirection = rTransform.GetTranslation() - pCamera->GetMatrix().GetTranslation();
			vecDirection.Normalise();

			TVector4 vecOffset;
			vecOffset.Multiply( vecDirection, 0.5f );

			rTransform.m_f41 = rTransform.m_f41 - vecOffset.x;
			rTransform.m_f42 = rTransform.m_f42 - vecOffset.y;
			rTransform.m_f43 = rTransform.m_f43 - vecOffset.z;
		}
	}
	else if ( HASANYFLAG( m_eFlags, FLAGS_FOLLOW_CAMERA ) )
	{
		// Stay at the near plane of the camera
		ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

		TVector4 vecForward = pCamera->GetMatrix().AsBasisVector4( 2 );
		vecForward.w        = 1.0f;

		TVector4 vecOffset;
		vecOffset.Multiply( vecForward, ARenderer::GetSingleton()->GetNear() );

		rTransform.GetTranslation() = pCamera->GetMatrix().GetTranslation() + vecOffset;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00650b20
void AParticleEffectHandle::AttachToBone( AModelInstance* a_pModel, TINT a_iBone, TBOOL a_bFollowPosition, TBOOL a_bFollowRotation )
{
	m_pAttachModel = a_pModel;
	m_iAttachBone  = a_iBone;
	m_eFlags       = ( a_bFollowRotation ? FLAGS_FOLLOW_ROTATION : 0 ) | ( a_bFollowPosition != TFALSE ) | m_eFlags;

	TMatrix44 mBone;
	a_pModel->GetBoneTransform( a_iBone, mBone );

	TMatrix44& rTransform = m_pInstance->GetTransform();

	if ( HASANYFLAG( m_eFlags, FLAGS_FOLLOW_POSITION ) )
		rTransform.GetTranslation() = mBone.GetTranslation();

	if ( HASANYFLAG( m_eFlags, FLAGS_FOLLOW_ROTATION ) )
	{
		rTransform.AsBasisVector4( 0 ) = mBone.AsBasisVector4( 0 );
		rTransform.AsBasisVector4( 1 ) = mBone.AsBasisVector4( 1 );
		rTransform.AsBasisVector4( 2 ) = mBone.AsBasisVector4( 2 );
		rTransform.RotateX( -TMath::HALF_PI );
	}
}

// $Barnyard: FUNCTION 00650e70
void AParticleEffectHandle::AttachToCamera()
{
	m_eFlags |= FLAGS_FOLLOW_CAMERA;
	m_pInstance->GetTransform().GetTranslation() = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();
}

// $Barnyard: FUNCTION 00650fe0
AParticleEffectManager::AParticleEffectManager()
{
	AParticleSystemConfig oConfig;
	oConfig.iNumFloats    = 10880;
	oConfig.iNumVectors   = 5440;
	oConfig.iNumBytes     = 512;
	oConfig.iNumDwords    = 9600;
	oConfig.iNumVectors2  = 512;
	oConfig.iNumInstances = 256;

	m_pParticleSystem = new AParticleSystem( oConfig );
	m_pParticleSystem->SetMaterialLibraryCallbacks( OnMaterialLibraryLoaded, OnMaterialLibraryUnloaded );

	m_pEffectPool = m_pParticleSystem->GetEffectPool();
	m_iNumHandles = m_pEffectPool->GetMaxInstances();
	m_pHandles    = new AParticleEffectHandle[ m_iNumHandles ];

	for ( TINT i = 0; i < m_iNumHandles; i++ )
		m_oFreeHandles.InsertTail( &m_pHandles[ i ] );
}

// $Barnyard: FUNCTION 00651140
// $Barnyard: FUNCTION 00650ed0
AParticleEffectManager::~AParticleEffectManager()
{
	m_oFreeHandles.TGenericDList::RemoveAll();

	if ( m_pHandles )
		delete[] m_pHandles;

	if ( m_pParticleSystem )
		delete m_pParticleSystem;
}

// $Barnyard: FUNCTION 00651160
TBOOL AParticleEffectManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	TTask::OnUpdate( a_fDeltaTime );

	if ( !ms_bActive )
		return TFALSE;

	AShadowManager::GetSingleton()->Update();

	for ( auto it = m_oUsedHandles.Begin(); it != m_oUsedHandles.End(); it++ )
		it->Update( a_fDeltaTime );

	m_pParticleSystem->Update( a_fDeltaTime );

	return TTRUE;
}

// $Barnyard: FUNCTION 00650f70
void AParticleEffectManager::OnActivate()
{
	ms_bActive = TTRUE;
}

// $Barnyard: FUNCTION 00650f80
void AParticleEffectManager::OnDeactivate()
{
	ms_bActive = TFALSE;
}

// $Barnyard: FUNCTION 00650fb0
TINT AParticleEffectManager::FindEffect( const TCHAR* a_szName )
{
	return GetSingleton()->m_pEffectPool->FindEffect( a_szName );
}

// $Barnyard: FUNCTION 00650fc0
AParticleEffect* AParticleEffectManager::CloneEffect( TINT a_iEffect )
{
	return GetSingleton()->m_pEffectPool->GetEffect( a_iEffect )->Clone();
}

// $Barnyard: FUNCTION 006513b0
AParticleEffectHandle* AParticleEffectManager::PlayEffect( TINT a_iEffect, const TVector4& a_rcPosition, const TVector4& a_rcDirection )
{
	return SpawnEffect( m_pEffectPool->GetEffect( a_iEffect ), a_rcPosition, a_rcDirection );
}

// $Barnyard: FUNCTION 00651320
AParticleEffectHandle* AParticleEffectManager::SpawnEffect( AParticleEffect* a_pEffect, const TVector4& a_rcPosition, const TVector4& a_rcDirection )
{
	AParticleEffectInstance* pInstance = m_pEffectPool->CreateInstance( a_pEffect );

	if ( !pInstance )
		return TNULL;

	TMatrix44& rTransform       = pInstance->GetTransform();
	rTransform.GetTranslation() = a_rcPosition;
	rTransform.LookAtDirection( a_rcDirection, TVector4::VEC_NEGY );
	rTransform.RotateX( -TMath::HALF_PI );

	AParticleEffectHandle* pHandle = m_oFreeHandles.Tail();
	pHandle->Remove();
	pHandle->m_pInstance = pInstance;
	m_oUsedHandles.InsertTail( pHandle );

	pInstance->SetOnFinishedCallback( OnEffectFinished, pHandle );
	return pHandle;
}

// $Barnyard: FUNCTION 00651230
void AParticleEffectManager::OnEffectFinished( AParticleEffectInstance* a_pInstance, void* a_pUserData )
{
	AParticleEffectHandle*  pHandle  = TSTATICCAST( AParticleEffectHandle, a_pUserData );
	AParticleEffectManager* pManager = GetSingleton();

	if ( pHandle->m_fnOnFinished )
		pHandle->m_fnOnFinished( pHandle, pHandle->m_pOnFinishedUserData );

	pHandle->Remove();
	pHandle->m_pInstance           = TNULL;
	pHandle->m_fnOnFinished        = TNULL;
	pHandle->m_pOnFinishedUserData = TNULL;
	pHandle->m_pAttachModel        = TNULL;
	pHandle->m_iAttachBone         = -1;
	pHandle->m_eFlags              = AParticleEffectHandle::FLAGS_NONE;

	pManager->m_oFreeHandles.InsertTail( pHandle );
}

// $Barnyard: FUNCTION 00650f90
void AParticleEffectManager::UnloadParticleLibrary()
{
	AParticleSystem* pSystem = GetSingleton()->m_pParticleSystem;

	pSystem->KillAll();
	pSystem->UnloadLibrary();
}

// $Barnyard: FUNCTION 00650c10
void AParticleEffectManager::KillAll()
{
	m_pParticleSystem->KillAll();
}

// $Barnyard: FUNCTION 006511e0
void AParticleEffectManager::DetachAllEffects()
{
	for ( auto it = m_oUsedHandles.Begin(); it != m_oUsedHandles.End(); it++ )
		it->Detach();
}

// $Barnyard: FUNCTION 00650eb0
void AParticleEffectManager::OnMaterialLibraryLoaded( AMaterialLibrary* a_pLibrary )
{
	AMaterialLibraryManager::GetSingleton()->CreateTextures( a_pLibrary );
}

// $Barnyard: FUNCTION 00650ec0
void AParticleEffectManager::OnMaterialLibraryUnloaded( AMaterialLibrary* a_pLibrary )
{
	AMaterialLibraryManager::GetSingleton()->UnloadTexturesOfLibrary( a_pLibrary );
}
