#pragma once
#include "Render/ASkinShader/ASkinMesh.h"

#include <Render/TRenderPacket.h>
#include <Render/TSkeleton.h>
#include <Toshi/TNodeList.h>
#include <Platform/DX8/TRenderInterface_DX8.h>
#include <Platform/DX8/TVertexBlockResource_DX8.h>
#include <Platform/DX8/TVertexPoolResource_DX8.h>
#include <Platform/DX8/TIndexBlockResource_DX8.h>
#include <Platform/DX8/TIndexPoolResource_DX8.h>

#include <d3d8.h>
#include <xmmintrin.h>

//-----------------------------------------------------------------------------
// The low end (no vertex shader 1.1) path of ASkinShaderHAL and AFXShaderHAL:
// the meshes are skinned on the CPU into dynamic vertex buffers and drawn with
// the fixed function pipeline.
// NOTE: Retail has two identical copies of this code (ASkinShaderHAL 005f40f0,
// 005f1ce0, 005f3060 and AFXShaderHAL 005fbfa0, 005f9fc0, 005fb340) that only
// differ in member offsets and their static data. The shared code lives here,
// each class keeps its own (marked) member functions and statics.
//-----------------------------------------------------------------------------
namespace ASkinLowEnd
{

// Vertex of the skin meshes' vertex pool (stream 0)
struct SkinVertex
{
	Toshi::TVector3 Position;      // 0x00
	Toshi::TVector3 Normal;        // 0x0C
	TUINT8          aWeights[ 4 ]; // 0x18
	TUINT8          aBones[ 4 ];   // 0x1C, register index (bone index in the sub mesh * 3)
	Toshi::TVector2 UV;            // 0x20
};
TSTATICASSERT( sizeof( SkinVertex ) == 40 );

// Vertex written by the CPU skinning (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)
struct Vertex
{
	Toshi::TVector3 Position; // 0x00
	Toshi::TVector3 Normal;   // 0x0C
	Toshi::TVector2 UV;       // 0x18
};
TSTATICASSERT( sizeof( Vertex ) == 32 );

// Dynamic vertex buffer the skinned vertices are written to
struct VertexBuffer : Toshi::TNodeList<VertexBuffer>::TNode
{
	// NOTE: Folded with the AWorldShaderHAL one
	// $Barnyard: FUNCTION 005f6a10
	~VertexBuffer()
	{
		if ( pVertexBuffer )
		{
			pVertexBuffer->Release();
			pVertexBuffer = TNULL;
		}
	}

	IDirect3DVertexBuffer8* pVertexBuffer; // 0x0C
};

constexpr DWORD FVF        = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1; // 274
constexpr DWORD STATIC_FVF = 466;
constexpr UINT  VB_SIZE    = 256000;

#ifdef BARNYARD_COMMUNITY_PATCH
// NOTE: Retail locks with D3DLOCK_NOOVERWRITE, which never fails, so the same vertex buffer is
// rewritten for every skinned mesh while the GPU may still be reading it (characters get the
// vertices of other characters with hardware vertex processing). Discarding gives a fresh buffer.
constexpr DWORD LOCK_FLAGS = D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK;
#else
constexpr DWORD LOCK_FLAGS = D3DLOCK_NOOVERWRITE | D3DLOCK_NOSYSLOCK;
#endif

// The default material and light of the low end path
constexpr D3DMATERIAL8 DEFAULT_MATERIAL = {
	{ 1.0f, 1.0f, 1.0f, 1.0f }, // Diffuse
	{ 1.0f, 1.0f, 1.0f, 1.0f }, // Ambient
	{ 0.0f, 0.0f, 0.0f, 0.0f }, // Specular
	{ 0.0f, 0.0f, 0.0f, 0.0f }, // Emissive
	0.0f                        // Power
};

constexpr D3DLIGHT8 DEFAULT_LIGHT = {
	D3DLIGHT_DIRECTIONAL,
	{ 1.0f, 1.0f, 1.0f, 1.0f }, // Diffuse
	{ 1.0f, 1.0f, 1.0f, 1.0f }, // Specular
	{ 0.0f, 0.0f, 0.0f, 0.0f }, // Ambient
	{ 0.0f, -100.0f, 0.0f },    // Position
	{ 1.0f, 1.0f, 1.0f },       // Direction
	2000.0f,                    // Range
	1.0f,                       // Falloff
	0.0f,                       // Attenuation0
	0.0f,                       // Attenuation1
	0.0f,                       // Attenuation2
	0.0f,                       // Theta
	0.0f                        // Phi
};

// Transforms a vertex by a (blended) bone matrix
TFORCEINLINE void TransformVertex( Vertex* a_pDst, const SkinVertex* a_pSrc, const TFLOAT* a_pMatrix, TFLOAT a_fNormalW )
{
	const TFLOAT* m = a_pMatrix;

	a_pDst->Position.x = a_pSrc->Position.x * m[ 0 ] + a_pSrc->Position.y * m[ 4 ] + a_pSrc->Position.z * m[ 8 ] + m[ 12 ];
	a_pDst->Position.y = a_pSrc->Position.x * m[ 1 ] + a_pSrc->Position.y * m[ 5 ] + a_pSrc->Position.z * m[ 9 ] + m[ 13 ];
	a_pDst->Position.z = a_pSrc->Position.x * m[ 2 ] + a_pSrc->Position.y * m[ 6 ] + a_pSrc->Position.z * m[ 10 ] + m[ 14 ];

	a_pDst->Normal.x = a_pSrc->Normal.x * m[ 0 ] + a_pSrc->Normal.y * m[ 4 ] + a_pSrc->Normal.z * m[ 8 ] + a_fNormalW;
	a_pDst->Normal.y = a_pSrc->Normal.x * m[ 1 ] + a_pSrc->Normal.y * m[ 5 ] + a_pSrc->Normal.z * m[ 9 ] + a_fNormalW;
	a_pDst->Normal.z = a_pSrc->Normal.x * m[ 2 ] + a_pSrc->Normal.y * m[ 6 ] + a_pSrc->Normal.z * m[ 10 ] + a_fNormalW;

	a_pDst->UV = a_pSrc->UV;
}

// Skins every vertex of the mesh with up to 4 bones
template <class TMeshHAL>
void SkinVertices( Toshi::TRenderPacket* a_pRenderPacket, const TBYTE* a_pSrcVertices, TBYTE* a_pDstVertices, TBOOL a_bUseSSE, TFLOAT a_fNormalW )
{
	auto          pMesh        = TSTATICCAST( TMeshHAL, a_pRenderPacket->GetMesh() );
	auto          pBones       = a_pRenderPacket->GetSkeletonInstance()->GetBones();
	auto          uiNumVerts   = pMesh->GetVertexPool()->GetNumVertices();
	auto          pSrc         = TREINTERPRETCAST( const SkinVertex*, a_pSrcVertices );
	auto          pDst         = TREINTERPRETCAST( Vertex*, a_pDstVertices );
	TUINT         uiSubMesh    = 0;
	ASkinSubMesh* pSubMesh     = pMesh->GetSubMesh( 0 );
	TUINT         uiSubMeshEnd = pSubMesh->uiNumVertices;

	auto GetBoneMatrix = [ & ]( TUINT8 a_uiRegister ) -> const TFLOAT* {
		return TREINTERPRETCAST( const TFLOAT*, &pBones[ pSubMesh->aBones[ a_uiRegister / 3 ] ].m_Transform );
	};

	for ( TUINT i = 0; i < uiNumVerts; i++, pSrc++, pDst++ )
	{
		if ( uiSubMeshEnd == i )
		{
			// The vertices of the next sub mesh start here
			pSubMesh     = pMesh->GetSubMesh( ++uiSubMesh );
			uiSubMeshEnd = pSubMesh->uiNumVertices;
		}

		TFLOAT fWeight1 = pSrc->aWeights[ 0 ] * ( 1.0f / 255.0f );

		if ( fWeight1 == 1.0f )
		{
			// Only one bone
			TransformVertex( pDst, pSrc, GetBoneMatrix( pSrc->aBones[ 0 ] ), a_fNormalW );
			continue;
		}

		TFLOAT fWeight2 = pSrc->aWeights[ 1 ] * ( 1.0f / 255.0f );
		TFLOAT fWeight3 = pSrc->aWeights[ 2 ] * ( 1.0f / 255.0f );

		alignas( 16 ) TFLOAT aMatrix[ 16 ];

		if ( fWeight3 == 0.0f )
		{
			// Two bones
			const TFLOAT* pBone1 = GetBoneMatrix( pSrc->aBones[ 0 ] );
			const TFLOAT* pBone2 = GetBoneMatrix( pSrc->aBones[ 1 ] );

			if ( !a_bUseSSE )
			{
				for ( TINT k = 0; k < 16; k++ )
					aMatrix[ k ] = fWeight1 * pBone1[ k ] + fWeight2 * pBone2[ k ];
			}
			else
			{
				__m128 vWeight1 = _mm_set1_ps( fWeight1 );
				__m128 vWeight2 = _mm_set1_ps( fWeight2 );

				for ( TINT k = 0; k < 16; k += 4 )
					_mm_storeu_ps( &aMatrix[ k ], _mm_add_ps( _mm_mul_ps( _mm_loadu_ps( &pBone1[ k ] ), vWeight1 ), _mm_mul_ps( _mm_loadu_ps( &pBone2[ k ] ), vWeight2 ) ) );
			}
		}
		else
		{
			TFLOAT fWeight4 = pSrc->aWeights[ 3 ] * ( 1.0f / 255.0f );

			const TFLOAT* pBone1 = GetBoneMatrix( pSrc->aBones[ 0 ] );
			const TFLOAT* pBone2 = GetBoneMatrix( pSrc->aBones[ 1 ] );
			const TFLOAT* pBone3 = GetBoneMatrix( pSrc->aBones[ 2 ] );

			if ( fWeight4 == 0.0f )
			{
				// Three bones
				for ( TINT k = 0; k < 16; k++ )
					aMatrix[ k ] = ( fWeight1 * pBone1[ k ] + fWeight2 * pBone2[ k ] ) + fWeight3 * pBone3[ k ];
			}
			else
			{
				// Four bones
				const TFLOAT* pBone4 = GetBoneMatrix( pSrc->aBones[ 3 ] );

				for ( TINT k = 0; k < 16; k++ )
					aMatrix[ k ] = ( ( fWeight1 * pBone1[ k ] + fWeight2 * pBone2[ k ] ) + fWeight3 * pBone3[ k ] ) + fWeight4 * pBone4[ k ];
			}
		}

		TransformVertex( pDst, pSrc, aMatrix, a_fNormalW );
	}
}

// Transforms every vertex by the first bone it's attached to
template <class TMeshHAL>
void TransformVerticesRigid( Toshi::TRenderPacket* a_pRenderPacket, const TBYTE* a_pSrcVertices, TBYTE* a_pDstVertices, TFLOAT a_fNormalW )
{
	auto          pMesh        = TSTATICCAST( TMeshHAL, a_pRenderPacket->GetMesh() );
	auto          pBones       = a_pRenderPacket->GetSkeletonInstance()->GetBones();
	auto          uiNumVerts   = pMesh->GetVertexPool()->GetNumVertices();
	auto          pSrc         = TREINTERPRETCAST( const SkinVertex*, a_pSrcVertices );
	auto          pDst         = TREINTERPRETCAST( Vertex*, a_pDstVertices );
	TUINT         uiSubMesh    = 0;
	ASkinSubMesh* pSubMesh     = pMesh->GetSubMesh( 0 );
	TUINT         uiSubMeshEnd = pSubMesh->uiNumVertices;

	for ( TUINT i = 0; i < uiNumVerts; i++, pSrc++, pDst++ )
	{
		if ( uiSubMeshEnd == i )
		{
			pSubMesh     = pMesh->GetSubMesh( ++uiSubMesh );
			uiSubMeshEnd = pSubMesh->uiNumVertices;
		}

		TransformVertex( pDst, pSrc, TREINTERPRETCAST( const TFLOAT*, &pBones[ pSubMesh->aBones[ pSrc->aBones[ 0 ] / 3 ] ].m_Transform ), a_fNormalW );
	}
}

// Renders a skinned mesh with the fixed function pipeline.
// a_pShader->SkinVerticesCPU() and TShaderHAL::TransformVerticesRigid() write the vertices,
// a_bRigid is the m_Unk4 flag of the shader (always transform by the first bone).
template <class TShaderHAL, class TMeshHAL>
void Render( TShaderHAL* a_pShader, Toshi::TRenderPacket* a_pRenderPacket, Toshi::TNodeList<VertexBuffer>& a_rVertexBuffers, D3DMATERIAL8& a_rMaterial, D3DLIGHT8& a_rLight, const Toshi::TMatrix44& a_rWorldView, TBOOL a_bRigid )
{
	TOSHI_NAMESPACE_USING

	if ( !a_pRenderPacket ) return;

	auto pMesh = TSTATICCAST( TMeshHAL, a_pRenderPacket->GetMesh() );
	if ( !pMesh || pMesh->GetNumSubMeshes() == 0 ) return;

	auto pRenderInterface = TRenderD3DInterface::Interface();
	auto pD3DDevice       = pRenderInterface->GetDirect3DDevice();
	auto pBones           = a_pRenderPacket->GetSkeletonInstance()->GetBones();

	// Does any of the sub meshes use more than one bone?
	TBOOL bSkinOnCPU = TFALSE;

	for ( TUINT i = 0; i < pMesh->GetNumSubMeshes(); i++ )
	{
		if ( pMesh->GetSubMesh( i )->uiNumBones > 1 )
		{
			bSkinOnCPU = TTRUE;
			pD3DDevice->SetTransform( D3DTS_WORLD, a_pRenderPacket->GetModelViewMatrix() );
			break;
		}
	}

	// Setup lighting
	TVector3 vLightDir = TVector3( -a_pRenderPacket->GetLightDirection().x, -a_pRenderPacket->GetLightDirection().y, -a_pRenderPacket->GetLightDirection().z );
	TVector3 vLightDirView;
	TMatrix44::RotateVector( vLightDirView, a_rWorldView, vLightDir );

	a_rLight.Direction.x = vLightDirView.x;
	a_rLight.Direction.y = vLightDirView.y;
	a_rLight.Direction.z = vLightDirView.z;

	const TVector3& rLightColour = a_pRenderPacket->GetLightColour();

	if ( rLightColour.x > 1.0f || rLightColour.y > 1.0f || rLightColour.z > 1.0f )
	{
		a_rLight.Diffuse.r = rLightColour.x * ( 1.0f / 255.0f );
		a_rLight.Diffuse.g = rLightColour.y * ( 1.0f / 255.0f );
		a_rLight.Diffuse.b = rLightColour.z * ( 1.0f / 255.0f );
	}
	else
	{
		a_rLight.Diffuse.r = rLightColour.x;
		a_rLight.Diffuse.g = rLightColour.y;
		a_rLight.Diffuse.b = rLightColour.z;
	}

	a_rLight.Ambient.r = a_pRenderPacket->GetAmbientColour().x;
	a_rLight.Ambient.g = a_pRenderPacket->GetAmbientColour().y;
	a_rLight.Ambient.b = a_pRenderPacket->GetAmbientColour().z;

	pD3DDevice->SetMaterial( &a_rMaterial );
	pD3DDevice->SetLight( 0, &a_rLight );
	pD3DDevice->LightEnable( 0, TRUE );

	TVertexBlockResource::HALBuffer vertexHALBuffer;
	auto                            pVertexPool = TSTATICCAST( TVertexPoolResource, pMesh->GetVertexPool() );

	TIndexBlockResource::HALBuffer indexHALBuffer;

	if ( !bSkinOnCPU )
	{
		// Every sub mesh is attached to a single bone, let the fixed function pipeline transform them
		pD3DDevice->SetVertexShader( STATIC_FVF );
		pVertexPool->GetHALBuffer( &vertexHALBuffer );
		pD3DDevice->SetStreamSource( 0, vertexHALBuffer.apVertexBuffers[ 0 ], sizeof( SkinVertex ) );

		for ( TUINT i = 0; i < pMesh->GetNumSubMeshes(); i++ )
		{
			ASkinSubMesh* pSubMesh = pMesh->GetSubMesh( i );

			TMatrix44 oWorld;
			oWorld.Multiply( a_pRenderPacket->GetModelViewMatrix(), pBones[ pSubMesh->aBones[ 0 ] ].m_Transform );
			pD3DDevice->SetTransform( D3DTS_WORLD, oWorld );

			auto pIndexPool = TSTATICCAST( TIndexPoolResource, pSubMesh->pIndexPool );
			pIndexPool->GetHALBuffer( &indexHALBuffer );

			pD3DDevice->SetIndices( indexHALBuffer.pIndexBuffer, vertexHALBuffer.uiVertexOffset );
			pD3DDevice->DrawIndexedPrimitive(
			    D3DPT_TRIANGLESTRIP,
			    0,
			    pVertexPool->GetNumVertices(),
			    indexHALBuffer.uiIndexOffset,
			    pIndexPool->GetNumIndices() - 2
			);
		}

		return;
	}

	// Find a dynamic vertex buffer that can be locked without waiting for the GPU
	VertexBuffer* pBuffer      = TNULL;
	BYTE*         pDstVertices = TNULL;

	for ( auto it = a_rVertexBuffers.Begin(); it != a_rVertexBuffers.End(); )
	{
		BYTE* pLocked;

		if ( it->pVertexBuffer && it->pVertexBuffer->Lock( 0, sizeof( Vertex ) * pVertexPool->GetNumVertices(), &pLocked, LOCK_FLAGS ) >= 0 )
		{
			pDstVertices = pLocked;

			if ( pLocked )
			{
				// Move it to the end of the list
				pBuffer = it;
				a_rVertexBuffers.Remove( pBuffer );
				a_rVertexBuffers.InsertTail( pBuffer );
				break;
			}
		}
		else
		{
			pDstVertices = TNULL;
		}

		it = it->Next();
	}

	if ( !pDstVertices )
	{
		// Create a new one
		pBuffer = new VertexBuffer;

		if ( pBuffer )
		{
			if ( pRenderInterface->GetDirect3DDevice()->CreateVertexBuffer( VB_SIZE, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, FVF, D3DPOOL_DEFAULT, &pBuffer->pVertexBuffer ) < 0 )
				pBuffer->pVertexBuffer = TNULL;
		}

		if ( !pBuffer ) return;

		BYTE* pLocked;

		if ( !pBuffer->pVertexBuffer ||
		     pBuffer->pVertexBuffer->Lock( 0, sizeof( Vertex ) * pVertexPool->GetNumVertices(), &pLocked, LOCK_FLAGS ) < 0 ||
		     ( pDstVertices = pLocked, pLocked == TNULL ) )
		{
			delete pBuffer;
			return;
		}

		a_rVertexBuffers.InsertHead( pBuffer );
	}

	const TBYTE* pSrcVertices = pVertexPool->GetManagedStream( 0 );

	if ( a_bRigid || pMesh->GetNumSubMeshes() <= 1 )
		TShaderHAL::TransformVerticesRigid( a_pRenderPacket, pSrcVertices, pDstVertices );
	else
		a_pShader->SkinVerticesCPU( a_pRenderPacket, pSrcVertices, pDstVertices );

	pBuffer->pVertexBuffer->Unlock();
	pD3DDevice->SetVertexShader( FVF );
	pD3DDevice->SetStreamSource( 0, pBuffer->pVertexBuffer, sizeof( Vertex ) );

	for ( TUINT i = 0; i < pMesh->GetNumSubMeshes(); i++ )
	{
		auto pIndexPool = TSTATICCAST( TIndexPoolResource, pMesh->GetSubMesh( i )->pIndexPool );
		pIndexPool->GetHALBuffer( &indexHALBuffer );

		pD3DDevice->SetIndices( indexHALBuffer.pIndexBuffer, 0 );
		pD3DDevice->DrawIndexedPrimitive(
		    D3DPT_TRIANGLESTRIP,
		    0,
		    pVertexPool->GetNumVertices(),
		    indexHALBuffer.uiIndexOffset,
		    pIndexPool->GetNumIndices() - 2
		);
	}
}

} // namespace ASkinLowEnd
