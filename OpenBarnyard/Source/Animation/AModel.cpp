#include "pch.h"
#include "AModel.h"

#include "Render/AGlowViewport.h"
#include "Render/AStaticInstanceShader/AStaticInstanceMesh.h"
#include "Render/AStaticInstanceShader/AStaticInstanceMaterial.h"
#include "Render/ARenderer.h"
#include "Assets/AModelLoader.h"
#include "Assets/AMaterialLibraryManager.h"
#include "Helpers/ASkinLightingManager.h"
#include "Helpers/AInstanceLightingHelper.h"
#include "Render/ASkinShader/ASkinMaterial.h"

#include "Render/ABackend.h"

#include <Render/TRenderInterface.h>
#include <Toshi/T2String8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0060ffd0, CreateObject 006111e0, CreateObjectInPlace 00611240
TDEFINE_CLASS_INIT( AModelInstance );

// Rotation by 90 degrees around the Z axis, set up by AModelInstance::InitialiseClass
// NOTE: Nothing else references it in retail (0x0079b3c0).
static TMatrix44 s_matRotateZ90;

// $Barnyard: FUNCTION 0060ffe0
void AModelInstance::InitialiseClass()
{
	TQuaternion quatRotation = TQuaternion::IDENTITY;
	quatRotation.RotateAroundAxis( TVector3::VEC_POSZ, TMath::HALF_PI );
	s_matRotateZ90.SetFromQuaternion( quatRotation );
}

// DOES_NOTHING
void AModelInstance::DeinitialiseClass()
{
}

// $Barnyard: FUNCTION 0060fef0
AModel::AModel( const TPString8& a_rName, TTRB* a_pTRB )
    : m_Name( a_rName )
    , m_uiID( ms_uiNumCreated )
    , m_pManagedModel( TNULL )
    , m_Vec1( TVector3::VEC_ZERO )
    , m_Vec2( TVector3::VEC_ZERO )
{
	m_pManagedModel = AModel::Create( a_rName, a_pTRB );

	ms_uiNumCreated += 1;
}

// $Barnyard: FUNCTION 0060fc90
// $Barnyard: FUNCTION 0060fed0
AModel::~AModel()
{
	if ( m_pManagedModel )
	{
		delete m_pManagedModel;
	}
}

// $Barnyard: FUNCTION 0060f9f0
void AModel::Update( TFLOAT a_fDeltaTime )
{
	TRenderContext* pRenderCtx = ARenderer::GetSingleton()->GetMainViewport()->GetRenderContext();

	for ( TINT i = 0; i < m_vecInstanceRefs.Size(); i++ )
	{
		AModelInstance* pInstance    = m_vecInstanceRefs[ i ].Get();
		TSceneObject*   pSceneObject = pInstance->GetSceneObject();
		TModel*         pModel       = pSceneObject->GetInstance()->GetModel();

		auto& transform      = pSceneObject->GetTransform();
		auto& transformScale = transform.GetScale();
		auto& lod            = pModel->GetLOD( 0 );

		TVector4 vecBoundingSphere;
		vecBoundingSphere.w = lod.BoundingSphere.GetRadius() * TMath::Max( TMath::Max( transformScale.x, transformScale.y ), transformScale.z );

		if ( vecBoundingSphere.w <= 0.0f )
		// The instance's scale is zero, do not render
		{
			pInstance->SetVisible( TFALSE );
		}
		else
		// Check if the bounding sphere currently visible
		{
			TMatrix44 transformMatrix;
			transform.GetLocalMatrixImp( transformMatrix );

			TMatrix44::TransformVector( vecBoundingSphere.AsVector3(), transformMatrix, lod.BoundingSphere.GetOrigin() );

			if ( TRenderContext::CullSphereToFrustumSimple( vecBoundingSphere, pRenderCtx->GetWorldPlanes(), 6 ) )
			{
				// The object is visible, update clip flags to match the flags it was checked against
				pInstance->SetVisible( TTRUE );
				pInstance->SetClipFlags( pRenderCtx->GetClipFlags() );
			}
			else
			{
				// The object is not visible
				pInstance->SetVisible( TFALSE );
			}
		}

		if ( pInstance->IsUpdatingSkeleton() )
		{
			pInstance->GetSceneObject()->Update( a_fDeltaTime );
		}
	}
}

// $Barnyard: FUNCTION 0060fb10
void AModel::Render( TBOOL a_bGlow )
{
	for ( TINT i = 0; i < m_vecInstanceRefs.Size(); i++ )
	{
		auto& pModelInstance = m_vecInstanceRefs[ i ];
		auto  eFlags         = pModelInstance->m_eFlags;

		if ( HASANYFLAG( eFlags, AModelInstance::Flags_Glow ) == a_bGlow &&
		     HASANYFLAG( eFlags, AModelInstance::Flags_Visible ) &&
		     HASANYFLAG( eFlags, AModelInstance::Flags_UpdatingSkeleton ) )
		{
			auto pSceneObject = pModelInstance->GetSceneObject();
			auto pModel       = pSceneObject->GetInstance()->GetModel();

			auto& transform      = pSceneObject->GetTransform();
			auto& transformScale = transform.GetScale();
			auto& lod            = pModel->GetLOD( 0 );

			TFLOAT fRadiusScale = TMath::Max( TMath::Max( transformScale.x, transformScale.y ), transformScale.z );
			TFLOAT fRadius      = lod.BoundingSphere.GetRadius() * fRadiusScale;

			TMatrix44 transformMatrix;
			transform.GetLocalMatrixImp( transformMatrix );

			TVector3 boundingPos;
			TMatrix44::TransformVector( boundingPos, transformMatrix, lod.BoundingSphere.GetOrigin() );
			pSceneObject->Render( pModelInstance->GetClipFlags(), boundingPos );
		}
	}
}

// $Barnyard: FUNCTION 0060fd20
AModelInstanceRef AModel::CreateInstance()
{
	TString8      instanceName = GenerateInstanceName( m_Name );
	TSceneObject* pSceneObject = m_pManagedModel->CreateSceneObject( instanceName );

	AModelInstanceRef modelInstanceRef = new AModelInstance(
	    this,
	    pSceneObject,
	    TFALSE
	);

	m_vecInstanceRefs.PushBack( modelInstanceRef );
	return modelInstanceRef;
}

// $Barnyard: FUNCTION 0060fe40
void AModel::DestroyInstance( AModelInstance* a_pInstance )
{
	m_vecInstanceRefs.FindAndEraseFast( a_pInstance );
}

TSIZE AModel::GetNumInstances() const
{
	return m_vecInstanceRefs.Size();
}

// $Barnyard: FUNCTION 0060f8c0
TManagedModel* AModel::Create( const TPString8& a_rFilePath, TTRB* a_pTRB )
{
	// NOTE: retail also gets the name and never uses it
	TString8 modelName;
	GetNameFromPath( a_rFilePath, modelName );

	auto pModelPtr = new TManagedModel();

	if ( pModelPtr )
	{
		TString8 filepath = a_rFilePath.GetString8();
		filepath.MakeLower();

		if ( filepath.Find( ".trb" ) < 0 &&
		     filepath.Find( ".trz" ) < 0 )
		{
			filepath += ".trb";
		}

		pModelPtr->Create( filepath, a_pTRB );

		ASkinLightingManager::GetSingleton()->ApplySkinLight( pModelPtr, TNULL, TNULL );
	}

	return pModelPtr;
}

// $Barnyard: FUNCTION 0060f6e0
void AModel::GetNameFromPath( const TPString8& a_FilePath, TString8& a_rName )
{
	TString8 name = a_FilePath.GetString8();

	if ( name.Length() <= 8 )
	{
		a_rName = name;
	}
	else
	{
		TINT iPos1 = 0;
		TINT iPos2;

		do {
			iPos2 = iPos1;
			iPos1 = name.Find( '\\', iPos1 + 1 );
		} while ( -1 < iPos1 );

		do {
			iPos1 = iPos2;
			iPos2 = name.Find( '/', iPos1 + 1 );
		} while ( -1 < iPos2 );

		if ( iPos1 < 0 || name.Length() - 1 <= iPos1 ) iPos1 = 0;
		iPos1 += 1;
		iPos2 = name.Find( '_', iPos1 );
		if ( iPos2 < 0 ) iPos2 = name.Length() - 1;

		TINT iSize = iPos2 - iPos1;

		if ( iSize < 1 )
		{
			iPos1 = 0;
			iSize = 8;
		}
		else if ( 8 < iSize )
		{
			iSize = 8;
		}

		a_rName.Copy( name.GetString( iPos1 ), iSize );
	}
}

// $Barnyard: FUNCTION 0060f7f0
TString8 AModel::GenerateInstanceName( const TPString8& a_FilePath )
{
	static TUINT ms_uiDebugObjectIndex = 0;

	TString8 name;
	GetNameFromPath( a_FilePath, name );

	TString8 debugName;
	debugName.Format( "%s%i", name.GetString(), ms_uiDebugObjectIndex % 1000 );
	ms_uiDebugObjectIndex += 1;

	return debugName;
}

// $Barnyard: FUNCTION 00611280
AModelInstance::AModelInstance( AModel* a_pModel, TSceneObject* a_pSceneObject, TBOOL a_bEnableSkeletonUpdate )
    : m_ChangeEmitter( this )
{
	m_Scale        = TVector4( 1.0f, 1.0f, 1.0f, 1.0f );
	m_fLightValue  = 1.0f;
	m_fShadeValue  = 0.0f;
	m_pModel       = a_pModel;
	m_uiClipFlags  = TRenderContext::CLIPFLAGS_ALL;
	m_pSceneObject = a_pSceneObject;
	m_eFlags       = Flags_ReceivesLight | Flags_UpdatingSkeleton;

	SetSkeletonUpdating( a_bEnableSkeletonUpdate );
	m_pSceneObject->GetInstance()->SetCustomRenderMethod( RenderInstanceCallback, this );
}

// $Barnyard: FUNCTION 00610820
AModelInstance::AModelInstance()
    : m_ChangeEmitter( this )
{
	m_Scale            = TVector4( 1.0f, 1.0f, 1.0f, 1.0f );
	m_pModel           = TNULL;
	m_fShadeValue      = 0.0f;
	m_pSceneObject     = TNULL;
	m_uiClipFlags      = TRenderContext::CLIPFLAGS_ALL;
	m_fLightValue      = 1.0f;
	m_iLightingSection = -1;
	m_eFlags           = Flags_Glow | Flags_ReceivesLight;
}

// $Barnyard: FUNCTION 00610030
// $Barnyard: FUNCTION 006108a0
AModelInstance::~AModelInstance()
{
	m_pSceneObject->Delete();
	m_pSceneObject = TNULL;
}

// $Barnyard: FUNCTION 006100d0
void AModelInstance::SetTransform( const TMatrix44& a_rcTransform )
{
	m_pSceneObject->GetTransform().SetMatrix( a_rcTransform );
}

// $Barnyard: FUNCTION 00610b90
void AModelInstance::SetTransform( const TVector4& a_rcPosition, const TVector4& a_rcDirection )
{
	TMatrix44 matTransform;
	CreateRotationFromDirection( matTransform, a_rcDirection, TVector4::VEC_NEGY );
	matTransform.GetTranslation() = a_rcPosition;

	m_pSceneObject->GetTransform().SetMatrix( matTransform );
}

// $Barnyard: FUNCTION 00610be0
void AModelInstance::SetTransform( const TVector4& a_rcPosition, const TVector4& a_rcDirection, const TVector4& a_rcFallback )
{
	TMatrix44 matTransform;
	CreateRotationFromDirection( matTransform, a_rcDirection, TVector4::VEC_NEGY, a_rcFallback );
	matTransform.GetTranslation() = a_rcPosition;

	m_pSceneObject->GetTransform().SetMatrix( matTransform );
}

// $Barnyard: FUNCTION 00610c60
void AModelInstance::SetTransformUnusedUp( const TVector4& a_rcPosition, const TVector4& a_rcDirection, const TVector4& a_rcUnused )
{
	TMatrix44 matTransform;
	CreateRotationFromDirection( matTransform, a_rcDirection, TVector4::VEC_NEGY );
	matTransform.GetTranslation() = a_rcPosition;

	m_pSceneObject->GetTransform().SetMatrix( matTransform );
}

// $Barnyard: FUNCTION 00611330
void AModelInstance::SetTransformKeepRight( const TVector4& a_rcPosition, const TVector4& a_rcDirection )
{
	TMatrix44 matTransform;
	GetTransform().GetLocalMatrixImp( matTransform );

	TVector4 vecRight = matTransform.AsBasisVector4( BASISVECTOR_RIGHT );
	SetTransform( a_rcPosition, a_rcDirection, vecRight );
}

// Rotates a_rcVector by a_rcQuaternion (w of the result is 1)
// $Barnyard: FUNCTION 006c9740
void RotateVector( TVector4& a_rOutVector, const TQuaternion& a_rcQuaternion, const TVector4& a_rcVector )
{
	TQuaternion quatConjugate( -a_rcQuaternion.x, -a_rcQuaternion.y, -a_rcQuaternion.z, a_rcQuaternion.w );
	TQuaternion quatVector( a_rcVector.x, a_rcVector.y, a_rcVector.z, 0.0f );

	TQuaternion quatResult = a_rcQuaternion;
	quatResult *= quatVector;
	quatResult *= quatConjugate;

	a_rOutVector.Set( quatResult.x, quatResult.y, quatResult.z, 1.0f );
}

// $Barnyard: FUNCTION 00610140
void AModelInstance::SetTransformTilted( const TVector4& a_rcPosition, const TVector4& a_rcDirection, TFLOAT a_fPitch, TFLOAT a_fRoll )
{
	TVector4 vecForward = a_rcDirection;

	TVector4 vecRight;
	vecRight.CrossProduct( vecForward, TVector4::VEC_NEGY );
	vecRight.Normalise();

	// Pitch the direction around the right axis
	TQuaternion quatRotation = TQuaternion::IDENTITY;
	quatRotation.RotateAroundAxis( vecRight.AsVector3(), -a_fPitch );
	RotateVector( vecForward, quatRotation, vecForward );

	// Roll the right axis around the direction
	quatRotation = TQuaternion::IDENTITY;
	quatRotation.RotateAroundAxis( vecForward.AsVector3(), a_fRoll );
	RotateVector( vecRight, quatRotation, vecRight );

	TVector4 vecUp;
	vecUp.CrossProduct( vecRight, vecForward );

	vecForward.Negate3();
	vecRight.Negate3();

	TMatrix44 matTransform        = TMatrix44::IDENTITY;
	matTransform.m_f11            = vecRight.x;
	matTransform.m_f12            = vecRight.y;
	matTransform.m_f13            = vecRight.z;
	matTransform.m_f21            = vecForward.x;
	matTransform.m_f22            = vecForward.y;
	matTransform.m_f23            = vecForward.z;
	matTransform.m_f31            = vecUp.x;
	matTransform.m_f32            = vecUp.y;
	matTransform.m_f33            = vecUp.z;
	matTransform.GetTranslation() = a_rcPosition;

	m_pSceneObject->GetTransform().SetMatrix( matTransform );
}

// $Barnyard: FUNCTION 00610330
void AModelInstance::SetTranslation( const TVector3& a_rcTranslation )
{
	m_pSceneObject->GetTransform().SetTranslate( a_rcTranslation );
}

// $Barnyard: FUNCTION 00610e80
void AModelInstance::SetDirection( const TVector4& a_rcDirection )
{
	TVector4 vecPosition;
	GetPosition( vecPosition );

	TMatrix44 matTransform;
	CreateRotationFromDirection( matTransform, a_rcDirection, TVector4::VEC_NEGY );
	matTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ) = vecPosition;

	m_pSceneObject->GetTransform().SetMatrix( matTransform );
}

// $Barnyard: FUNCTION 00610f10
void AModelInstance::SetDirection( const TVector4& a_rcDirection, const TVector4& a_rcFallback )
{
	const TVector3& rcTranslation = GetTransform().GetTranslation();
	TVector4        vecPosition( rcTranslation.x, rcTranslation.y, rcTranslation.z, 1.0f );

	SetTransform( vecPosition, a_rcDirection, a_rcFallback );
}

// $Barnyard: FUNCTION 00610790
TBOOL AModelInstance::IsFacingDifferent( const TVector4& a_rcDirection ) const
{
	TMatrix44 matTransform;
	GetTransform().GetLocalMatrixImp( matTransform );

	TVector4 vecFacing( -matTransform.m_f21, -matTransform.m_f22, -matTransform.m_f23, 1.0f );
	return !a_rcDirection.IsEqual( vecFacing );
}

// $Barnyard: FUNCTION 00610340
void AModelInstance::GetPosition( TVector4& a_rOutPosition ) const
{
	const TVector3& rcTranslation = GetTransform().GetTranslation();
	a_rOutPosition.Set( rcTranslation.x, rcTranslation.y, rcTranslation.z, 1.0f );
}

// $Barnyard: FUNCTION 006100e0
void AModelInstance::SetTransform( const TVector4& a_rcPosition, const TQuaternion& a_rcRotation )
{
	TMatrix44 matTransform = TMatrix44::IDENTITY;
	matTransform.SetFromQuaternion( a_rcRotation );
	matTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ) = a_rcPosition;

	m_pSceneObject->GetTransform().SetMatrix( matTransform );
}

// $Barnyard: FUNCTION 00610740
void AModelInstance::GetUpVector( TVector4& a_rOutUp ) const
{
	TMatrix44 matTransform;
	GetTransform().GetLocalMatrixImp( matTransform );

	a_rOutUp = matTransform.AsBasisVector4( BASISVECTOR_UP );
}

// $Barnyard: FUNCTION 00611070
void AModelInstance::SetEnvironmentMap( const TCHAR* a_szTextureName, TINT a_iNumExcluded, const TCHAR* const* a_ppExcluded )
{
	// NOTE: the original calls SetUnlit() if RETURNS_FALSE (00660960) returns TRUE for the object returned by
	// the virtual at +0x20 of the render interface, so never

	TModel* pModel = m_pSceneObject->GetModelRef()->GetModel();

	for ( TINT i = 0; i < pModel->GetNumLODs(); i++ )
	{
		TModelLOD& rLOD = pModel->GetLOD( i );

		for ( TINT k = 0; k < rLOD.iNumMeshes; k++ )
		{
			TMaterial* pMaterial = rLOD.ppMeshes[ k ]->GetMaterial();

			if ( !pMaterial->GetClass()->IsA( &TGetClass( ASkinMaterial ) ) )
				continue;

			TBOOL bExcluded = TFALSE;

			for ( TINT j = 0; j < a_iNumExcluded; j++ )
			{
				if ( TStringManager::String8FindString( pMaterial->GetName(), a_ppExcluded[ j ] ) )
				{
					bExcluded = TTRUE;
					break;
				}
			}

			if ( !bExcluded )
			{
				TSTATICCAST( ASkinMaterialHAL, pMaterial )->SetSomeTexture( AMaterialLibraryManager::GetSingleton()->FindTexture( a_szTextureName ) );
			}
		}
	}
}

// $Barnyard: FUNCTION 00610ff0
void AModelInstance::SetUnlit()
{
	SetShadowMode( SHADOWMODE_UNLIT );

	TModel* pModel = m_pSceneObject->GetModelRef()->GetModel();

	for ( TINT i = 0; i < pModel->GetNumLODs(); i++ )
	{
		TModelLOD& rLOD = pModel->GetLOD( i );

		for ( TINT k = 0; k < rLOD.iNumMeshes; k++ )
		{
			TMaterial* pMaterial = rLOD.ppMeshes[ k ]->GetMaterial();

			if ( pMaterial->GetClass()->IsA( &TGetClass( ASkinMaterial ) ) )
			{
				TSTATICCAST( ASkinMaterialHAL, pMaterial )->SetHDLighting( TFALSE );
				pMaterial->SetTextureNum( 1 );
			}
		}
	}
}

// $Barnyard: FUNCTION 00610800
void AModelInstance::SetShadowMode( SHADOWMODE a_uiMode )
{
	m_eFlags = a_uiMode | ( m_eFlags & ~SHADOWMODE_MASK );
}

// $Barnyard: FUNCTION 00611170
void AModelInstance::UpdateLighting()
{
	if ( ( m_eFlags & SHADOWMODE_MASK ) == SHADOWMODE_DEFAULT )
	{
		TVector4 vecPosition;
		GetPosition( vecPosition );

		AInstanceLightingHelper* pLightingHelper = AInstanceLightingHelper::GetSingleton();
		m_fShadeValue                            = pLightingHelper->GetShadeValue( vecPosition );
		m_fLightValue                            = pLightingHelper->GetLightValue( vecPosition );
	}
}

// $Barnyard: FUNCTION 006104a0
void AModelInstance::CreateRotationFromDirection( TMatrix44& a_rOutMatrix, const TVector4& a_rcDirection, const TVector4& a_rcUp )
{
	TVector4 vecForward;
	vecForward.Negate3( a_rcDirection );

	TVector4 vecRight;
	vecRight.CrossProduct( vecForward, a_rcUp );
	vecRight.Normalise();

	TVector4 vecUp;
	vecUp.CrossProduct( vecRight, vecForward );
	vecUp.Normalise();

	a_rOutMatrix.Identity();
	a_rOutMatrix.m_f11 = vecRight.x;
	a_rOutMatrix.m_f12 = vecRight.y;
	a_rOutMatrix.m_f13 = vecRight.z;
	a_rOutMatrix.m_f21 = vecForward.x;
	a_rOutMatrix.m_f22 = vecForward.y;
	a_rOutMatrix.m_f23 = vecForward.z;
	a_rOutMatrix.m_f31 = vecUp.x;
	a_rOutMatrix.m_f32 = vecUp.y;
	a_rOutMatrix.m_f33 = vecUp.z;
}

// $Barnyard: FUNCTION 00610570
void AModelInstance::CreateRotationFromDirection( TMatrix44& a_rOutMatrix, const TVector4& a_rcDirection, const TVector4& a_rcUp, const TVector4& a_rcFallback )
{
	TVector4 vecUp        = a_rcUp;
	TBOOL    bUseFallback = TFALSE;

	TFLOAT fDot = TVector4::DotProduct3( a_rcDirection, vecUp );

	if ( TMath::Abs( fDot - 1.0f ) <= TMath::TFLOAT_EPSILON || TMath::Abs( fDot + 1.0f ) <= TMath::TFLOAT_EPSILON )
	{
		// The direction is parallel to the up vector, can't use it
		vecUp        = a_rcFallback;
		bUseFallback = TTRUE;
	}

	TVector4 vecForward;
	vecForward.Negate3( a_rcDirection );

	TVector4 vecRight;
	vecRight.CrossProduct( vecForward, vecUp );
	vecRight.Normalise();

	TVector4 vecNewUp;
	vecNewUp.CrossProduct( vecRight, vecForward );
	vecNewUp.Normalise();

	a_rOutMatrix.Identity();

	if ( bUseFallback )
	{
		a_rOutMatrix.m_f11 = vecForward.x;
		a_rOutMatrix.m_f12 = vecForward.y;
		a_rOutMatrix.m_f13 = vecForward.z;
		a_rOutMatrix.m_f21 = vecRight.x;
		a_rOutMatrix.m_f22 = vecRight.y;
		a_rOutMatrix.m_f23 = vecRight.z;
	}
	else
	{
		a_rOutMatrix.m_f11 = vecRight.x;
		a_rOutMatrix.m_f12 = vecRight.y;
		a_rOutMatrix.m_f13 = vecRight.z;
		a_rOutMatrix.m_f21 = vecForward.x;
		a_rOutMatrix.m_f22 = vecForward.y;
		a_rOutMatrix.m_f23 = vecForward.z;
	}

	a_rOutMatrix.m_f31 = vecNewUp.x;
	a_rOutMatrix.m_f32 = vecNewUp.y;
	a_rOutMatrix.m_f33 = vecNewUp.z;
}

// $Barnyard: FUNCTION 006108c0
void AModelInstance::RenderInstanceCallback( TModelInstance* a_pInstance, void* a_pUserData )
{
	TModel*            pModel            = a_pInstance->GetModel();
	TSkeletonInstance* pSkeletonInstance = a_pInstance->GetSkeletonInstance();

	AModelInstance* pGameModelInstance = TSTATICCAST( AModelInstance, a_pUserData );

	// Update skeleton state
	pSkeletonInstance->UpdateState( TTRUE );

	// Update current render context with actual skeleton instance to render
	TRenderContext* pRenderContext = TRenderInterface::GetSingleton()->GetCurrentContext();
	pRenderContext->SetSkeletonInstance( pSkeletonInstance );

	TModelLOD* pLOD         = &pModel->GetLOD( a_pInstance->GetLOD() );
	TBOOL      bIsWorldMesh = TFALSE;

	// Check if it's a world model
	if ( pLOD->iNumMeshes > 0 )
	{
#ifdef BARNYARD_COMMUNITY_PATCH
		bIsWorldMesh = pLOD->ppMeshes[ 0 ]->GetClass()->IsA( &TGetClass( AWorldShader ) );
#else  // BARNYARD_COMMUNITY_PATCH
		bIsWorldMesh = pLOD->ppMeshes[ 0 ]->GetClass() == &TGetClass( AWorldShaderHAL );
#endif // !BARNYARD_COMMUNITY_PATCH 
	}

	// Check if object can be affected by any light sources
	pRenderContext->ClearLightIDs();
	if ( pLOD->iNumMeshes > 0 && pGameModelInstance->ReceivesLight() )
	{
		TMatrix44 matTransform;
		pGameModelInstance->GetSceneObject()->GetTransform().GetLocalMatrixImp( matTransform );

		// Get position of bounding sphere transformed by this scene object
		TSphere boundingSphere = pLOD->BoundingSphere;
		TMatrix44::TransformVector( boundingSphere.GetOrigin(), matTransform, boundingSphere.GetOrigin() );

		TLightIDList lightIds;
		AGlowViewport::GetSingleton()->GetInfluencingLightIDs( boundingSphere, lightIds );

		if ( lightIds[ 0 ] != -1 )
		{
			// Add light to the render context
			pRenderContext->AddLight( lightIds[ 0 ] );
		}
	}

	static TUINT s_uiUnusedFlags;
	if ( !( s_uiUnusedFlags & 1 ) )
		s_uiUnusedFlags |= 1;

	static TMatrix44 s_oOldModelViewMatrix;
	TMatrix44        oScaledModelViewMatrix;

	if ( pGameModelInstance )
	{
		// Save current model view matrix
		s_oOldModelViewMatrix = pRenderContext->GetModelViewMatrix();

		// Calculate model view matrix for this instance and set it
		oScaledModelViewMatrix = pRenderContext->GetModelViewMatrix();
		oScaledModelViewMatrix.Scale( pGameModelInstance->GetScale() );

		pRenderContext->SetModelViewMatrix( oScaledModelViewMatrix );
	}

	// Don't do anything if there are no meshed to render
	if ( pLOD->iNumMeshes == 0 )
		return;

	TBOOL                    bIsStaticInstanceMesh = pLOD->ppMeshes[ 0 ]->IsA( &TGetClass( AStaticInstanceMesh ) );
	AStaticInstanceMaterial* pStaticInstanceMat    = AModelLoader::ms_pDefaultStaticInstanceMaterial;
	AWorldMaterial*          pWorldMat             = AModelLoader::ms_pDefaultWorldMaterial;
	ASkinMaterial*           pSkinMat              = AModelLoader::ms_pDefaultSkinMaterial;

	static constexpr TINT MAX_NUM_MATERIALS = 32;
	TMaterial*            apOldMaterials[ MAX_NUM_MATERIALS ];
	// NOTE: only the default material passes use apOldMaterials (the farmer's truck has more meshes)
	TASSERT( !pGameModelInstance->DrawWithDefaultMaterials() || pLOD->iNumMeshes < MAX_NUM_MATERIALS );

	if ( !pGameModelInstance->DrawWithDefaultMaterials() )
	{
		for ( TINT i = 0; i < pLOD->iNumMeshes; i++ )
		{
			pLOD->ppMeshes[ i ]->Render();
		}
	}
	else if ( bIsStaticInstanceMesh )
	{
		// Render meshes with the default material
		for ( TINT i = 0; i < pLOD->iNumMeshes; i++ )
		{
			apOldMaterials[ i ] = pLOD->ppMeshes[ i ]->GetMaterial();
			pLOD->ppMeshes[ i ]->SetMaterial( pStaticInstanceMat );
			pLOD->ppMeshes[ i ]->Render();
		}

		// Render meshes with their original materials
		for ( TINT i = 0; i < pLOD->iNumMeshes; i++ )
		{
			pLOD->ppMeshes[ i ]->SetMaterial( apOldMaterials[ i ] );
			pLOD->ppMeshes[ i ]->Render();
		}
	}
	else if ( bIsWorldMesh )
	{
		// Render meshes with the default material
		for ( TINT i = 0; i < pLOD->iNumMeshes; i++ )
		{
			apOldMaterials[ i ] = pLOD->ppMeshes[ i ]->GetMaterial();
			pLOD->ppMeshes[ i ]->SetMaterial( pWorldMat );
			pLOD->ppMeshes[ i ]->Render();
		}

		// Render meshes with their original materials
		for ( TINT i = 0; i < pLOD->iNumMeshes; i++ )
		{
			pLOD->ppMeshes[ i ]->SetMaterial( apOldMaterials[ i ] );
			pLOD->ppMeshes[ i ]->Render();
		}
	}
	else
	{
		// Render meshes with the default material
		for ( TINT i = 0; i < pLOD->iNumMeshes; i++ )
		{
			apOldMaterials[ i ] = pLOD->ppMeshes[ i ]->GetMaterial();
			pLOD->ppMeshes[ i ]->SetMaterial( pSkinMat );
			pLOD->ppMeshes[ i ]->Render();
		}

		// Render meshes with their original materials
		for ( TINT i = 0; i < pLOD->iNumMeshes; i++ )
		{
			pLOD->ppMeshes[ i ]->SetMaterial( apOldMaterials[ i ] );
			pLOD->ppMeshes[ i ]->Render();
		}
	}

	if ( pGameModelInstance )
	{
		// Restore old model view matrix
		pRenderContext->SetModelViewMatrix( s_oOldModelViewMatrix );
	}
}

// $Barnyard: FUNCTION 00610430
void AModelInstance::GetBoneTransform( TINT a_iBone, TMatrix44& a_rOutTransform )
{
	TMatrix44 matBone;
	m_pSceneObject->GetInstance()->GetSkeletonInstance()->GetBoneTransformCurrent( a_iBone, matBone );

	// NOTE: retail scales the translation of a_rOutTransform, not of matBone, and
	// GetLocalMatrixImp overwrites it right after, so the scale is never applied
	a_rOutTransform.GetTranslation().Multiply( m_Scale );
	m_pSceneObject->GetTransform().GetLocalMatrixImp( a_rOutTransform );

	TMatrix44 matLocal = a_rOutTransform;
	a_rOutTransform.Multiply( matLocal, matBone );
}

// $Barnyard: FUNCTION 00610400
void AModelInstance::GetScaledBoneTransform( TINT a_iBone, TMatrix44& a_rOutTransform )
{
	m_pSceneObject->GetInstance()->GetSkeletonInstance()->GetBoneTransformCurrent( a_iBone, a_rOutTransform );
	a_rOutTransform.GetTranslation().Multiply( m_Scale );
}

// $Barnyard: FUNCTION 00610da0
TINT AModelInstance::GetBoneID( const TPString8& a_rcBoneName )
{
	const TString8& strBoneName = a_rcBoneName.GetString8();
	return GetSkeleton()->GetBoneID( strBoneName.GetString(), strBoneName.Length() );
}

// $Barnyard: FUNCTION 00610de0
TINT AModelInstance::GetBoneIndex( const TPString8& a_rcBoneName )
{
	const TString8& strBoneName = a_rcBoneName.GetString8();
	return GetSkeleton()->GetBoneID( strBoneName.GetString(), strBoneName.Length() );
}

// $Barnyard: FUNCTION 006103d0
const TCHAR* AModelInstance::GetBoneName( TINT a_iBone )
{
	return GetSkeleton()->GetBone( a_iBone )->GetName();
}

// $Barnyard: FUNCTION 00610f60
void AModelInstance::ApplyMaterial( const TCHAR* a_szTextureName, const TCHAR* a_szMaterialName, const TCHAR* a_szSkinLight )
{
	TManagedModel* pManagedModel = m_pSceneObject->GetModelRef();
	TModel*        pModel        = pManagedModel->GetModel();

	if ( pModel->GetNumLODs() > 0 )
	{
		TModelLOD& rLOD = pModel->GetLOD( 0 );

		for ( TINT i = 0; i < rLOD.iNumMeshes; i++ )
		{
			TMaterial* pMaterial = rLOD.ppMeshes[ i ]->GetMaterial();

			if ( pMaterial->GetTextureNum() != 0 && TStringManager::String8CompareNoCase( pMaterial->GetName(), a_szMaterialName ) == 0 )
			{
				AModelLoader::ChangeMaterialTexture( pMaterial, a_szTextureName );
			}
		}

		ASkinLightingManager* pSkinLightingManager = ASkinLightingManager::GetSingleton();

		if ( a_szSkinLight )
		{
			ASkinLightingManager::SkinLight* pSkinLight = pSkinLightingManager->FindSkinLight( a_szSkinLight );

			if ( pSkinLight )
				pSkinLightingManager->ApplySkinLight( pManagedModel, a_szMaterialName, pSkinLight );
		}
	}
}

// $Barnyard: FUNCTION 00610710
void AModelInstance::SetOpacity( TFLOAT a_fOpacity )
{
	m_pSceneObject->EnableOpacity();
	m_pSceneObject->GetFadeParams().y = a_fOpacity;
}

// $Barnyard: FUNCTION 00610700
void AModelInstance::ResetOpacity()
{
	m_pSceneObject->DisableOpacity();
}

// $Barnyard: FUNCTION 00610cd0
void AModelInstance::SetSkeletonUpdating( TBOOL a_bUpdating )
{
	if ( a_bUpdating )
	{
		if ( !IsUpdatingSkeleton() )
		{
			m_pSceneObject->EnableSkeletonUpdate();
			m_ChangeEmitter.Throw( ChangeEvent_EnabledSkeletonUpdate );
		}

		m_eFlags |= Flags_UpdatingSkeleton;
	}
	else
	{
		if ( IsUpdatingSkeleton() )
		{
			m_pSceneObject->DisableSkeletonUpdate();
			m_ChangeEmitter.Throw( ChangeEvent_DisabledSkeletonUpdate );
		}

		m_eFlags &= ~Flags_UpdatingSkeleton;
	}
}

// $Barnyard: FUNCTION 00610380
void AModelInstance::SetReceivesLight( TBOOL a_bReceives )
{
	if ( a_bReceives )
		m_eFlags |= Flags_ReceivesLight;
	else
		m_eFlags &= ~Flags_ReceivesLight;
}

// $Barnyard: FUNCTION 00610d40
void AModelInstance::SetVisible( TBOOL a_bVisible )
{
	if ( a_bVisible )
	{
		if ( !IsVisible() )
			m_ChangeEmitter.Throw( ChangeEvent_Visibility );

		m_eFlags |= Flags_Visible;
	}
	else
	{
		if ( IsVisible() )
			m_ChangeEmitter.Throw( ChangeEvent_Visibility );

		m_eFlags &= ~Flags_Visible;
	}
}

// $Barnyard: FUNCTION 006100c0
const TPString8& AModelInstance::GetModelName() const
{
	return m_pModel->GetName();
}
