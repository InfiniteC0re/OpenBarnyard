#include "pch.h"
#include "AModel.h"

#include "Render/AGlowViewport.h"
#include "Render/AStaticInstanceShader/AStaticInstanceMesh.h"
#include "Render/AStaticInstanceShader/AStaticInstanceMaterial.h"
#include "Assets/AModelLoader.h"

#ifdef TOSHI_SKU_WINDOWS
#  include "Platform/DX8/AWorldShader/AWorldShader_DX8.h"
#endif

#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AModelInstance );

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
	TIMPLEMENT();

	for ( TINT i = 0; i < m_vecInstanceRefs.Size(); i++ )
	{
		auto pInstance = m_vecInstanceRefs[ i ].Get();

		if ( pInstance->IsUpdatingSkeleton() )
		{
			pInstance->GetSceneObject()->Update( a_fDeltaTime );
		}
	}
}

// $Barnyard: FUNCTION 0060fb10
void AModel::Render( TUINT8 a_uiFlags )
{
	for ( TINT i = 0; i < m_vecInstanceRefs.Size(); i++ )
	{
		auto& pModelInstance = m_vecInstanceRefs[ i ];
		auto  eFlags         = pModelInstance->m_eFlags;

		if ( ( ( eFlags >> 4 & 1 ) == a_uiFlags && ( eFlags & 2 ) != 0 ) && ( eFlags & 1 ) != 0 )
		{
			auto pSceneObject = pModelInstance->GetSceneObject();
			auto pModel       = pSceneObject->GetInstance()->GetModel();

			auto& transform      = pSceneObject->GetTransform();
			auto& transformScale = transform.GetScale();
			auto& lod            = pModel->GetLOD( 0 );

			TFLOAT fRadiusScale = TMath::Max( TMath::Max( transformScale.x, transformScale.y ), transformScale.z );
			TFLOAT fRadius      = lod.BoundingSphere.GetRadius() * fRadiusScale;

			Toshi::TMatrix44 transformMatrix;
			transform.GetLocalMatrixImp( transformMatrix );

			Toshi::TVector3 boundingPos;
			Toshi::TMatrix44::TransformVector( boundingPos, transformMatrix, lod.BoundingSphere.GetOrigin() );
			pSceneObject->Render( pModelInstance->GetClipFlags(), boundingPos );
		}
	}
}

// $Barnyard: FUNCTION 0060fd20
AModelInstanceRef AModel::CreateInstance()
{
	// TODO: use this name in debug?
	TString8 instanceName = GenerateInstanceName( m_Name );

	AModelInstanceRef modelInstanceRef = new AModelInstance(
	    this,
	    m_pManagedModel->CreateSceneObject(),
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
	// TODO: use this name in debug?
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
		TFIXME( "Call FUN_0060dcc0" );
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

		TUINT iSize = iPos2 - iPos1;

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
	TFIXME( "Initialise some unknown members" );

	m_Scale        = TVector4( 1.0f, 1.0f, 1.0f, 1.0f );
	m_pModel       = a_pModel;
	m_uiClipFlags  = 0x3F;
	m_pSceneObject = a_pSceneObject;
	m_eFlags       = 0b00001001;

	SetSkeletonUpdating( a_bEnableSkeletonUpdate );
	m_pSceneObject->GetInstance()->SetCustomRenderMethod( RenderInstanceCallback, this );
}

// $Barnyard: FUNCTION 00610820
AModelInstance::AModelInstance()
    : m_ChangeEmitter( this )
{
	TFIXME( "Initialise some unknown members" );

	m_Scale        = TVector4( 1.0f, 1.0f, 1.0f, 1.0f );
	m_pModel       = TNULL;
	m_pSceneObject = TNULL;
	m_uiClipFlags  = 0x3F;
	m_eFlags       = 0b00011000;
}

// $Barnyard: FUNCTION 006100d0
void AModelInstance::SetTransform( const Toshi::TMatrix44& a_rcTransform )
{
	m_pSceneObject->GetTransform().SetMatrix( a_rcTransform );
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
		bIsWorldMesh = pLOD->ppMeshes[ 0 ]->GetClass() == &TGetClass( AWorldShaderHAL );
	}

	// Check if object can be affected by any light sources
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
	TASSERT( pLOD->iNumMeshes < MAX_NUM_MATERIALS );

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
