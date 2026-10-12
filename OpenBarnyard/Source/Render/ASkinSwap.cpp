#include "pch.h"
#include "ASkinSwap.h"
#include "Animation/AModel.h"
#include "Animation/AAnimatableObjectType.h"
#include "Assets/AMaterialLibraryManager.h"
#include "Render/ASkinShader/ASkinShader.h"

#include <Toshi/TSceneObject.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0060df80
ASkinSwap::ASkinSwap()
{
	m_fnOrigRenderCb    = TNULL;
	m_pSwapMaterial     = TNULL;
	m_pOriginalMaterial = TNULL;

	m_pSwapMaterial = ASkinShader::GetSingleton()->CreateMaterial( TNULL );
	m_pSwapMaterial->Create( 0 );
	m_pSwapMaterial->SetName( "swap" );
	m_pSwapMaterial->SetTextureNum( 1 );
}

// $Barnyard: FUNCTION 0060e390
ASkinSwap::~ASkinSwap()
{
	Reset();
}

// $Barnyard: FUNCTION 0060dfd0
void ASkinSwap::RenderCallback( TModelInstance* a_pInstance, void* a_pUserData )
{
	ASkinSwap*         pSwap   = TSTATICCAST( ASkinSwap, a_pUserData );
	AAnimatableObject* pObject = pSwap->m_pObject;

	AModelInstance* pModelInstance = pObject->GetModelInstance().Get();

	ASkinMaterial*                   pSwapMaterial     = pSwap->m_pSwapMaterial;
	TModelInstance::t_CustomRenderCB fnOrigRenderCb    = pSwap->m_fnOrigRenderCb;
	ASkinMaterial*                   pOriginalMaterial = pSwap->m_pOriginalMaterial;
	TModelLOD&                       rLOD              = pObject->GetModelInstance()->GetSceneObject()->GetInstance()->GetModel()->GetLOD( 0 );

	// Replace the original skin material with the swap one...
	for ( TINT i = 0; i < rLOD.iNumMeshes; i++ )
	{
		TMesh* pMesh = rLOD.ppMeshes[ i ];

		if ( pMesh->GetMaterial() == pOriginalMaterial )
			pMesh->SetMaterial( pSwapMaterial );
	}

	// ...render the model...
	fnOrigRenderCb( a_pInstance, pModelInstance );

	// NOTE: retail copies the LOD structure onto itself here (no effect)

	// ...and restore the original material
	for ( TINT i = 0; i < rLOD.iNumMeshes; i++ )
	{
		TMesh* pMesh = rLOD.ppMeshes[ i ];

		if ( pMesh->GetMaterial() == pSwapMaterial )
			pMesh->SetMaterial( pOriginalMaterial );
	}
}

// $Barnyard: FUNCTION 0060e100
void ASkinSwap::SetObject( AAnimatableObject* a_pObject, TINT a_iSkin )
{
	if ( m_fnOrigRenderCb )
	{
		// Restore render callback of the previous object
		AModelInstance* pModelInstance = m_pObject->GetModelInstance().Get();
		m_pObject->GetModelInstance()->GetSceneObject()->GetInstance()->SetCustomRenderMethod( m_fnOrigRenderCb, pModelInstance );
		m_fnOrigRenderCb = TNULL;
	}

	m_pObject = a_pObject;
	m_iSkin   = a_iSkin;

	const TCHAR* pchSkinTexture  = a_pObject->GetObjectType()->GetSkinTexture( a_iSkin );
	const TCHAR* pchMaterialName = m_pObject->GetObjectType()->GetMaterialName();

	if ( !pchMaterialName || m_pObject->GetObjectType()->GetNumSkins() == 1 )
		return;

	// Hook the render callback
	m_fnOrigRenderCb = AModelInstance::RenderInstanceCallback;
	m_pObject->GetModelInstance()->GetSceneObject()->GetInstance()->SetCustomRenderMethod( RenderCallback, this );

	// Find the skin material of the model
	TModelLOD& rLOD = a_pObject->GetModelInstance()->GetSceneObject()->GetInstance()->GetModel()->GetLOD( 0 );

	for ( TINT i = 0; i < rLOD.iNumMeshes; i++ )
	{
		TMaterial* pMaterial = rLOD.ppMeshes[ i ]->GetMaterial();

		if ( pMaterial->GetTextureNum() != 0 &&
		     TStringManager::String8CompareNoCase( pMaterial->GetName(), pchMaterialName ) == 0 &&
		     pMaterial->GetClass()->IsA( &TGetClass( ASkinMaterial ) ) )
		{
			// Copy the material's textures, but use the alternative skin texture
			m_pOriginalMaterial = TSTATICCAST( ASkinMaterial, pMaterial );
			m_pSwapMaterial->SetTextureNum( m_pOriginalMaterial->GetTextureNum() );

			for ( TUINT k = 1; k < m_pOriginalMaterial->GetTextureNum(); k++ )
				m_pSwapMaterial->m_apLightingTextures[ k - 1 ] = m_pOriginalMaterial->m_apLightingTextures[ k - 1 ];

			m_pSwapMaterial->m_pTexture = AMaterialLibraryManager::GetSingleton()->FindTexture( pchSkinTexture );
			return;
		}
	}
}

// $Barnyard: FUNCTION 0060e300
void ASkinSwap::Reset()
{
	if ( m_fnOrigRenderCb )
	{
		AModelInstance* pModelInstance = m_pObject->GetModelInstance().Get();
		m_pObject->GetModelInstance()->GetSceneObject()->GetInstance()->SetCustomRenderMethod( m_fnOrigRenderCb, pModelInstance );
		m_fnOrigRenderCb = TNULL;
	}

	if ( m_pSwapMaterial )
	{
		m_pSwapMaterial->Destroy();
		m_pSwapMaterial = TNULL;
	}
}
