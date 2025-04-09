#include "pch.h"
#include "ABoneAttachCameraHelper.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AModelRepos.h"

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
		TMatrix44 matBoneTransform;
		m_pSkeletonInstance->GetBoneTransformCurrent( m_iBoneId, matBoneTransform );
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

void ABoneAttachCameraHelper::OnDestroy()
{
	TASSERT( !"Not implemented" );
	//AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_pAnimatableObject );
	m_pAnimatableObject = TNULL;
}

void ABoneAttachCameraHelper::OnAttach()
{
	
}

// $Barnyard: FUNCTION 0045fd00
void ABoneAttachCameraHelper::OnDetach()
{
	m_oAnimStartListener.Disconnect();

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
