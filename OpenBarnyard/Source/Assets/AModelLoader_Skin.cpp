#include "pch.h"
#include "AModelLoader.h"

#ifdef TOSHI_SKU_WINDOWS
#  include "Platform/DX8/ASkinShader/ASkinMesh_DX8.h"
#  include "Platform/DX8/ASkinShader/ASkinShader_DX8.h"
#  include "Platform/DX8/ASkinShader/ASkinMaterial_DX8.h"
#endif // TOSHI_SKU_WINDOWS

#include <Toshi/T2String.h>
#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static TClass* s_pSkinMaterialClass = TNULL;

// $Barnyard: FUNCTION 006135d0
void AModelLoader::LoadSkinLOD( Toshi::TModel* a_pModel, TINT a_iLODIndex, Toshi::TModelLOD* a_pLOD, Toshi::TTMDWin::TRBLODHeader* a_pLODHeader )
{
	TPROFILER_SCOPE();

	auto pShader = TDYNAMICCAST( ASkinShaderHAL, ASkinShader::GetSingleton() );

	TINT iMeshCount = a_pLODHeader->m_iMeshCount1 + a_pLODHeader->m_iMeshCount2;

	for ( TINT i = 0; i < iMeshCount; i++ )
	{
		T2FormatString128 symbolName;
		symbolName.Format( "LOD%d_Mesh_%d", a_iLODIndex, i );

		auto pTRBMesh = a_pModel->CastSymbol<TTMDWin::TRBLODMesh>( symbolName.Get() );

		auto pMesh = pShader->CreateMesh( TNULL );
		pMesh->Create( 0, pTRBMesh->m_uiNumVertices, pTRBMesh->m_uiNumSubMeshes );

		if ( TNULL == s_pSkinMaterialClass )
		{
			s_pSkinMaterialClass = TClass::Find( "ASkinMaterial" );
		}

		auto pMaterial = CreateMaterial( pShader, pTRBMesh->m_szMaterialName );
		pMesh->SetMaterial( pMaterial );

		a_pLOD->ppMeshes[ i ] = pMesh;

		for ( TUINT k = 0; k < pTRBMesh->m_uiNumSubMeshes; k++ )
		{
			auto pSubMesh    = pMesh->GetSubMesh( k );
			auto pTRBSubMesh = &pTRBMesh->m_pSubMeshes[ k ];

			TUtil::MemCopy( pSubMesh->aBones, pTRBSubMesh->m_pBones, pTRBSubMesh->m_uiNumBones * sizeof( TINT ) );

			// Create index pool
			if ( pSubMesh->pIndexPool == TNULL )
			{
				auto pIndexFactory = TRenderInterface::GetSingleton()->GetSystemResource<TIndexFactoryResourceInterface>( SYSRESOURCE_IFSYS );
				TVALIDPTR( pIndexFactory );

				pSubMesh->pIndexPool = pIndexFactory->CreatePoolResource( pTRBMesh->m_uiNumIndices, 9 );
			}

			pSubMesh->uiNumBones    = pTRBSubMesh->m_uiNumBones;
			pSubMesh->uiNumVertices = pTRBSubMesh->m_uiNumVertices2;
			pSubMesh->Zero          = pTRBSubMesh->m_Zero;
			pSubMesh->Unk2          = pTRBSubMesh->m_Unk2;
			pSubMesh->Unk3          = pTRBSubMesh->m_Unk3;
			pSubMesh->Unk4          = pTRBSubMesh->m_Unk4;
			pSubMesh->Unk5          = pTRBSubMesh->m_Unk5;
			pSubMesh->Unk6          = pTRBSubMesh->m_Unk6;

			if ( pTRBSubMesh->m_uiNumVertices1 != 0 )
			{
				auto pVertexPool = pMesh->GetVertexPool();

				TVertexPoolResourceInterface::LockBuffer vertexLockBuffer;
				TBOOL                                    bLocked = pVertexPool->Lock( &vertexLockBuffer );

				TASSERT( bLocked && "Couldn't lock vertex buffer" );

				if ( bLocked )
				{
					TUtil::MemCopy( vertexLockBuffer.apStreams[ 0 ], pTRBSubMesh->m_pVertices, pTRBSubMesh->m_uiNumVertices1 * 40 );
					pVertexPool->Unlock( pTRBSubMesh->m_uiNumVertices1 );
				}
			}

			auto pIndexPool = pSubMesh->pIndexPool;
			TVALIDPTR( pIndexPool );

			if ( pTRBSubMesh->m_uiNumIndices != 0 )
			{
				TIndexPoolResourceInterface::LockBuffer indexLockBuffer;

				pIndexPool->Lock( &indexLockBuffer );
				TUtil::MemCopy( indexLockBuffer.pBuffer, pTRBSubMesh->m_pIndices, pTRBSubMesh->m_uiNumIndices * 2 );
				pIndexPool->Unlock( pTRBSubMesh->m_uiNumIndices );
			}
		}
	}
}
