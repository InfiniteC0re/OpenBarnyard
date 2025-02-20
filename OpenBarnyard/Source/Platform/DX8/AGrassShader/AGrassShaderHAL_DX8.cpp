#include "pch.h"
#include "AGrassShaderHAL_DX8.h"
#include "AGrassMaterialHAL_DX8.h"
#include "AGrassMeshHAL_DX8.h"
#include "Platform/DX8/AWorldShader/AWorldShader_DX8.h"

#include <Toshi/TScheduler.h>
#include <Platform/DX8/TRenderInterface_DX8.h>
#include <Platform/DX8/TRenderContext_DX8.h>
#include <Platform/DX8/TVertexBlockResource_DX8.h>
#include <Platform/DX8/TVertexPoolResource_DX8.h>
#include <Platform/DX8/TIndexBlockResource_DX8.h>
#include <Platform/DX8/TIndexPoolResource_DX8.h>
#include <Platform/DX8/TTextureResourceHAL_DX8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGrassShaderHAL );

TTextureResourceHAL* g_pGrassShaderTerrainTexture = TNULL;
static TVector3      g_vecAnimationDataOffset;
static TVector4      g_vecAnimOffset;
static const TFLOAT  g_fAnimationSpeed = 2.25f;

// $Barnyard: FUNCTION 005f7c10
AGrassShaderHAL::AGrassShaderHAL()
    : m_hVertexShader( 0 )
{
}

// $Barnyard: FUNCTION 005f7c70
// $Barnyard: FUNCTION 005f8310
AGrassShaderHAL::~AGrassShaderHAL()
{
}

// $Barnyard: FUNCTION 005f7ee0
void AGrassShaderHAL::Flush()
{
	Validate();

	TRenderD3DInterface* pRenderInterface = TRenderD3DInterface::Interface();
	TRenderContextD3D*   pCurrentContext  = TRenderContextD3D::Upcast( pRenderInterface->GetCurrentContext() );
	IDirect3DDevice8*    pD3DDevice       = pRenderInterface->GetDirect3DDevice();

	pD3DDevice->SetVertexShader( m_hVertexShader );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, 2 );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, 2 );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, 1 );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, 4 );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, 2 );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, 1 );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, 4 );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, 2 );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, 1 );
	pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, 1 );
	pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, 1 );
	pD3DDevice->SetVertexShader( D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZ );

	static TMatrix44 s_ViewMatrix = TMatrix44::IDENTITY;

	pD3DDevice->SetTransform( D3DTS_PROJECTION, pCurrentContext->GetProjectionMatrix() );
	pD3DDevice->SetTransform( D3DTS_VIEW, s_ViewMatrix );

	pD3DDevice->SetPixelShader( 0 );
	pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
	pD3DDevice->SetRenderState( D3DRS_ALPHAREF, 1 );
	pD3DDevice->SetRenderState( D3DRS_ALPHAFUNC, 5 );
	pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
	pD3DDevice->SetRenderState( D3DRS_LIGHTING, 0 );

	m_oOrderTable.Render();
	pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
}

// $Barnyard: FUNCTION 005f8660
void AGrassShaderHAL::StartFlush()
{
	if ( IsValidated() )
	{
		TRenderD3DInterface* pRenderInterface = TRenderD3DInterface::Interface();
		IDirect3DDevice8*    pD3DDevice       = pRenderInterface->GetDirect3DDevice();

		pD3DDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU, 1 );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV, 1 );
		pD3DDevice->SetVertexShader( m_hVertexShader );
		pD3DDevice->SetPixelShader( 0 );

		AWorldShaderHAL*          pWorldShader    = TSTATICCAST( AWorldShaderHAL, AWorldShader::GetSingleton() );
		TVector4                  vecShadowColor  = pWorldShader->GetShadowColour();
		TVector4                  vecAmbientColor = pWorldShader->GetAmbientColour();
		static constexpr TVector4 vecSomeColor    = TVector4( 1.0f, 1.0f, 1.0f, 1.0f );

		pD3DDevice->SetVertexShaderConstant( 4, &vecAmbientColor, 1 );
		pD3DDevice->SetVertexShaderConstant( 5, &vecShadowColor, 1 );
		pD3DDevice->SetVertexShaderConstant( 6, &vecSomeColor, 1 );

		pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
		pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
		pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
		pD3DDevice->SetRenderState( D3DRS_ALPHAFUNC, 5 );

		DWORD dwWasSpecularEnabled;
		pD3DDevice->GetRenderState( D3DRS_SPECULARENABLE, &dwWasSpecularEnabled );
		pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, 4 );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, 2 );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, 0 );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, 4 );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, 2 );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, 0 );
		pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, 1 );
		pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, 1 );

		UpdateAnimation();
	}
}

// $Barnyard: FUNCTION 005f7ea0
void AGrassShaderHAL::EndFlush()
{
	TRenderD3DInterface* pRenderInterface = TRenderD3DInterface::Interface();
	IDirect3DDevice8*    pD3DDevice       = pRenderInterface->GetDirect3DDevice();

	pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
	pD3DDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
}

// $Barnyard: FUNCTION 005f7cd0
TBOOL AGrassShaderHAL::Create()
{
	m_oOrderTable.Create( this, -2000 );
	AGrassShader::Create();

	return Validate();
}

// $Barnyard: FUNCTION 005f7d00
static TBOOL CreateGrassVertexShader( const char* a_szFileName, const DWORD* a_pFunction, DWORD* a_pOutVertexShader )
{
	static constexpr DWORD s_ShaderDeclaration[] = {
		0x20000000, 0x40020000, 0x40020001, 0x40020002,
		0x40010003, 0xFFFFFFFF
	};

	return TRenderD3DInterface::CreateVertexShader( s_ShaderDeclaration, a_pFunction, a_pOutVertexShader );
}

// $Barnyard: FUNCTION 005f7d60
TBOOL AGrassShaderHAL::Validate()
{
	constexpr static TUINT8 s_VertexShaderFunction[] = { 0x01, 0x01, 0xfe, 0xff, 0x05, 0x00, 0x00, 0x00, 0x03, 0x00, 0x0f, 0x80, 0x01, 0x00, 0xe4, 0x90, 0x07, 0x00, 0xe4, 0xa0, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x0f, 0x80, 0x08, 0x00, 0xe4, 0xa0, 0x05, 0x00, 0x00, 0x00, 0x05, 0x00, 0x0f, 0x80, 0x05, 0x00, 0xe4, 0x80, 0x07, 0x00, 0xe4, 0xa0, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x0f, 0x80, 0x03, 0x00, 0xe4, 0x80, 0x05, 0x00, 0xe4, 0x80, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x0f, 0x80, 0x03, 0x00, 0xe4, 0x80, 0x00, 0x00, 0xe4, 0x90, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x03, 0x00, 0xe4, 0x80, 0x00, 0x00, 0xe4, 0xa0, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0f, 0x80, 0x06, 0x00, 0xe4, 0xa0, 0x02, 0x00, 0xe4, 0x91, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x02, 0x00, 0xe4, 0x90, 0x04, 0x00, 0xe4, 0xa0, 0x05, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0f, 0x80, 0x01, 0x00, 0xe4, 0x80, 0x05, 0x00, 0xe4, 0xa0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0xe4, 0x80, 0x02, 0x00, 0xe4, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x80, 0x06, 0x00, 0xff, 0xa0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xd0, 0x00, 0x00, 0xe4, 0x80, 0x05, 0x00, 0x00, 0x00, 0x04, 0x00, 0x0f, 0x80, 0x03, 0x00, 0xe4, 0x90, 0x08, 0x00, 0xff, 0xa0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe0, 0x04, 0x00, 0xe4, 0x80, 0xff, 0xff, 0x00, 0x00 };

	if ( IsValidated() )
		return TTRUE;

	if ( !CreateGrassVertexShader( "AGrassShader_D3D8_Win.vsh", (DWORD*)s_VertexShaderFunction, &m_hVertexShader ) )
		return TFALSE;

	return AGrassShader::Validate();
}

// $Barnyard: FUNCTION 005f7da0
void AGrassShaderHAL::Invalidate()
{
	if ( IsValidated() )
	{
		if ( m_hVertexShader != 0 )
		{
			TRenderD3DInterface::DestroyVertexShader( m_hVertexShader );
		}

		AGrassShader::Invalidate();
	}
}

TBOOL AGrassShaderHAL::TryInvalidate()
{
	Invalidate();
	return TTRUE;
}

TBOOL AGrassShaderHAL::TryValidate()
{
	Validate();
	return TTRUE;
}

// $Barnyard: FUNCTION 005f8330
void AGrassShaderHAL::Render( Toshi::TRenderPacket* a_pRenderPacket )
{
	AGrassMeshHAL* pMesh;

	if ( a_pRenderPacket && ( pMesh = TSTATICCAST( AGrassMeshHAL, a_pRenderPacket->GetMesh() ), pMesh != TNULL ) )
	{
		TRenderD3DInterface* pRenderInterface = TRenderD3DInterface::Interface();
		TRenderContextD3D*   pCurrentContext  = TRenderContextD3D::Upcast( pRenderInterface->GetCurrentContext() );
		IDirect3DDevice8*    pD3DDevice       = pRenderInterface->GetDirect3DDevice();

		D3DXMATRIX matMVP;
		D3DXMatrixMultiply( &matMVP, a_pRenderPacket->GetModelViewMatrix(), pCurrentContext->GetProjectionMatrix() );
		D3DXMatrixTranspose( &matMVP, &matMVP );

		pD3DDevice->SetVertexShaderConstant( 0, &matMVP, 4 );

		TVertexBlockResource::HALBuffer oVertexBuffer;
		TVertexPoolResource*            pVertexPool = TSTATICCAST( TVertexPoolResource, pMesh->GetVertexPool() );
		pVertexPool->GetHALBuffer( &oVertexBuffer );
		pD3DDevice->SetStreamSource( 0, oVertexBuffer.apVertexBuffers[ 0 ], 44 );

		TIndexBlockResource::HALBuffer oIndexBuffer;
		TIndexPoolResource*            pIndexPool = TSTATICCAST( TIndexPoolResource, pMesh->GetSubMesh( 0 )->pIndexPool );
		pIndexPool->GetHALBuffer( &oIndexBuffer );
		pD3DDevice->SetIndices( oIndexBuffer.pIndexBuffer, oVertexBuffer.uiVertexOffset );

		TVector4 vecUnknown = TVector4( 0.0f, 0.0f, 0.0f, 0.0f );
		g_vecAnimOffset.w   = 1.0f;

		pD3DDevice->SetVertexShaderConstant( 8, &g_vecAnimOffset, 1 );
		pD3DDevice->SetVertexShaderConstant( 7, &vecUnknown, 1 );
		pD3DDevice->SetTexture( 0, g_pGrassShaderTerrainTexture->GetD3DTexture() );
		pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, 0, pVertexPool->GetNumVertices(), oIndexBuffer.uiIndexOffset, pIndexPool->GetNumIndices() - 2 );
	
		TTODO( "Draw grass layers" );
	}
}

// $Barnyard: FUNCTION 005f8b10
AGrassMaterial* AGrassShaderHAL::CreateMaterial( const TCHAR* a_szName )
{
	Validate();

	AGrassMaterialHAL* pMaterial = new AGrassMaterialHAL();
	pMaterial->SetShader( this );

	m_oOrderTable.RegisterMaterial( pMaterial );
	return pMaterial;
}

// $Barnyard: FUNCTION 005f8a80
AGrassMesh* AGrassShaderHAL::CreateMesh( const TCHAR* a_szName )
{
	Validate();

	AGrassMeshHAL* pMesh = new AGrassMeshHAL();
	pMesh->SetOwnerShader( this );

	return pMesh;
}

// $Barnyard: FUNCTION 005f7de0
void AGrassShaderHAL::UpdateAnimation()
{
	TFLOAT fCurrentTime = g_oSystemManager.GetScheduler()->GetTotalTime();

	TVector3 vecAnim = TVector3(
	    fCurrentTime * 0.70f * g_fAnimationSpeed,
	    fCurrentTime * 0.92f * g_fAnimationSpeed,
	    fCurrentTime * 0.20f * g_fAnimationSpeed
	);

	TMath::NormaliseAngle( vecAnim.x );
	TMath::NormaliseAngle( vecAnim.y );
	TMath::NormaliseAngle( vecAnim.z );

	g_vecAnimationDataOffset.x = TMath::Sin( vecAnim.x );
	g_vecAnimationDataOffset.y = TMath::Sin( vecAnim.y );
	g_vecAnimationDataOffset.z = TMath::Sin( vecAnim.z );

	g_vecAnimOffset.x = g_vecAnimationDataOffset.x;
	g_vecAnimOffset.y = g_vecAnimationDataOffset.y;
	g_vecAnimOffset.z = g_vecAnimationDataOffset.z;
}
