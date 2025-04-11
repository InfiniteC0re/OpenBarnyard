#include "pch.h"
#include "ABoneAttachCameraHelper.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AModelRepos.h"
#include "Render/ARenderer.h"

#include <Toshi/TScheduler.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( ABoneAttachCameraHelper )

// $Barnyard: FUNCTION 0045fca0
ABoneAttachCameraHelper::ABoneAttachCameraHelper()
    : m_pAnimation( TNULL )
    , m_pAModelInstance( TNULL )
    , m_pSceneObject( TNULL )
    , m_pSkeletonInstance( TNULL )
    , m_iBoneId( -1 )
    , m_pAnimatableObject( TNULL )
{

}

// $Barnyard: FUNCTION 0045fe30
// $Barnyard: FUNCTION 0045fdd0
ABoneAttachCameraHelper::~ABoneAttachCameraHelper()
{
	ThisClass::OnDetach();
}

// $Barnyard: FUNCTION 0045f990
TBOOL ABoneAttachCameraHelper::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_pSceneObject )
	{
		// Get bone transform transformed by the model transform
		TMatrix44 matBoneTransform = m_pSkeletonInstance->GetBoneTransformCurrent( m_iBoneId );
		m_pSceneObject->GetTransform().GetLocalMatrixImp( m_oTransform );

		m_oTransform.Multiply( matBoneTransform );

		TVector4 vecDirection = m_oTransform.AsBasisVector4( BASISVECTOR_FORWARD );
		vecDirection.Normalise();
		vecDirection.x = -vecDirection.x;
		vecDirection.y = -vecDirection.y;
		vecDirection.z = -vecDirection.z;
		vecDirection.w = 1.0f;

		m_pCamera->GetMatrix().SetTranslation( m_oTransform.GetTranslation() );
		m_pCamera->LookAtDirection( vecDirection );
	}

	return TTRUE;
}

TBOOL ABoneAttachCameraHelper::Unk2( void* )
{
	return TTRUE;
}

TBOOL ABoneAttachCameraHelper::Unk3( void*, void* )
{
	return TTRUE;
}

void ABoneAttachCameraHelper::OnCreate()
{
}

// $Barnyard: FUNCTION 0045fb10
void ABoneAttachCameraHelper::OnDestroy()
{
	AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_pAnimatableObject );
	m_pAnimatableObject = TNULL;
}

void ABoneAttachCameraHelper::OnAttach()
{
	
}

// $Barnyard: FUNCTION 0045fd00
void ABoneAttachCameraHelper::OnDetach()
{
	m_oAnimStartListener.Disconnect();

	if ( m_pSkeletonInstance )
	{
		m_pSkeletonInstance->RemoveAllAnimations();
	}

	if ( m_pAnimation )
	{
		m_iBoneId = -1;
		m_pAnimation = TNULL;
	}

	if ( m_pAModelInstance )
	{
		TPString8 strModelName = m_pAModelInstance->GetModelName();

		AModelRepos::GetSingleton()->DestroyModelInstance( m_pAModelInstance );
		AModelRepos::GetSingleton()->UnloadModel( strModelName, TFALSE );
		AModelRepos::GetSingleton()->UnloadUnusedModels();

		m_pSceneObject      = TNULL;
		m_pSkeletonInstance = TNULL;
		m_pAModelInstance   = TNULL;
	}
}

void ABoneAttachCameraHelper::BoneHelper_Unknown()
{
}

// $Barnyard: FUNCTION 0045fba0
void ABoneAttachCameraHelper::SetModel( const Toshi::TPString8& a_strModelName, const TCHAR* a_szBoneName )
{
	AModelRepos* pModelRepos = AModelRepos::GetSingleton();

	// Make sure the model is loaded
	pModelRepos->LoadModel( a_strModelName, TNULL );

	AModel* pModel = pModelRepos->GetModel( a_strModelName );
	TASSERT( TNULL != pModel );

	AModelInstance* pInstance = pModelRepos->InstantiateModel( pModel );
	TASSERT( TNULL != pInstance );

	m_pAModelInstance = pInstance;
	m_pAModelInstance->GetTransform().SetEuler( TVector3( TMath::HALF_PI, 0.0f, 0.0f ) );

	m_pSceneObject      = m_pAModelInstance->GetSceneObject();
	m_pSkeletonInstance = m_pSceneObject->GetSkeletonInstance();

	TString8 strBoneName = a_szBoneName;
	m_iBoneId = m_pSkeletonInstance->GetSkeleton()->GetBoneID( strBoneName, strBoneName.Length() );
	TASSERT( m_iBoneId != TBONE_INVALID );

	m_oAnimStartListener.Connect(
	    ARenderer::GetSingleton()->GetAnimationUpdateStartEmitter(),
	    this,
	    &AnimationStartEventHandler,
	    0
	);
}

// $Barnyard: FUNCTION 0045fa80
void ABoneAttachCameraHelper::SetAnimation( const TCHAR* a_szSeqName )
{
	if ( m_pAnimation )
	{
		m_pSkeletonInstance->RemoveAnimation( m_pAnimation, 0.0f );
		m_pAnimation = TNULL;
	}

	TINT iSeqId = m_pSkeletonInstance->GetSkeleton()->GetSequenceID( a_szSeqName );
	TASSERT( iSeqId != TSEQ_INVALID );

	m_pAnimation = m_pSkeletonInstance->AddAnimation( iSeqId, 1.0f, 0.0f );
}

// $Barnyard: FUNCTION 0045fb30
TBOOL __stdcall ABoneAttachCameraHelper::AnimationStartEventHandler( ABoneAttachCameraHelper* a_pCaller, ARenderer* a_pOwner, TBOOL** a_pData )
{
	TScheduler* pScheduler = g_oSystemManager.GetScheduler();

	a_pCaller->m_pSkeletonInstance->UpdateTime( pScheduler->GetCurrentDeltaTime() );
	a_pCaller->m_pSkeletonInstance->UpdateState( TTRUE );

	return TTRUE;
}
