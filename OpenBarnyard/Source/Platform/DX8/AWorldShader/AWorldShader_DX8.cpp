#include "pch.h"
#include "AWorldShader_DX8.h"
#include "AWorldMaterial_DX8.h"
#include "AWorldMesh_DX8.h"
#include "World/AWorld.h"

#include <Render/TRenderPacket.h>
#include <Platform/DX8/TRenderInterface_DX8.h>
#include <Platform/DX8/TRenderContext_DX8.h>
#include <Platform/DX8/TVertexBlockResource_DX8.h>
#include <Platform/DX8/TVertexPoolResource_DX8.h>
#include <Platform/DX8/TIndexBlockResource_DX8.h>
#include <Platform/DX8/TIndexPoolResource_DX8.h>
#include <d3d8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AWorldShaderHAL );

// $Barnyard: FUNCTION 005f75e0
AWorldShaderHAL::AWorldShaderHAL()
    : m_ShadowColour( 0.3f, 0.3f, 0.3f, 1.0f ), m_AmbientColour( 1.0f, 1.0f, 1.0f, 1.0f )
{
	m_hVertexShader = 0;
	m_pUnk2         = TNULL;
	m_iAlphaRef     = 128;

	m_bIsHighEndMode      = TTRUE;
	m_bAlphaBlendMaterial = TFALSE;
	m_bUnkFlag3           = TFALSE;
	m_bUnkFlag4           = TFALSE;
}

// $Barnyard: FUNCTION 005f77a0
AWorldShaderHAL::~AWorldShaderHAL()
{
}

// $Barnyard: FUNCTION 005f66f0
void AWorldShaderHAL::Flush()
{
	if ( IsValidated() )
	{
		auto pRenderInterface = TRenderD3DInterface::Interface();
		auto pCurrentContext  = TRenderContextD3D::Upcast( pRenderInterface->GetCurrentContext() );
		auto pDevice          = pRenderInterface->GetDirect3DDevice();

		if ( IsHighEndMode() )
		{
			Validate();
			pDevice->SetVertexShader( m_hVertexShader );
			pDevice->SetPixelShader( NULL );

			static constexpr TVector4 s_SomeVector( 1.0f, 1.0f, 1.0f, 1.0f );
			pDevice->SetVertexShaderConstant( 4, &m_AmbientColour, 1 );
			pDevice->SetVertexShaderConstant( 5, &m_ShadowColour, 1 );
			pDevice->SetVertexShaderConstant( 6, &s_SomeVector, 1 );

			pDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
			pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
			pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
			pDevice->SetRenderState( D3DRS_ALPHAFUNC, 5 );

			DWORD dwWasSpecularEnabled;
			pDevice->GetRenderState( D3DRS_SPECULARENABLE, &dwWasSpecularEnabled );
			pDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
			pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, 4 );
			pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, 2 );
			pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, 0 );
			pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, 4 );
			pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, 2 );
			pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, 0 );
			pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, 1 );
			pDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, 1 );
			pDevice->SetRenderState( D3DRS_CULLMODE, m_bRenderEnvMap ? D3DCULL_CCW : D3DCULL_CW );
			pDevice->SetRenderState( D3DRS_SPECULARENABLE, dwWasSpecularEnabled );
			pDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );
			pDevice->SetPixelShader( 0 );
		}
		else
		{
			SetupLowEndMode();
		}
	}
}

// $Barnyard: FUNCTION 005f6510
void AWorldShaderHAL::StartFlush()
{
	if ( IsValidated() )
	{
		auto pRenderInterface = TRenderD3DInterface::Interface();
		auto pCurrentContext  = TRenderContextD3D::Upcast( pRenderInterface->GetCurrentContext() );
		auto pDevice          = pRenderInterface->GetDirect3DDevice();

		pDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU, 1 );
		pDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV, 1 );

		if ( !IsHighEndMode() )
		{
			SetupLowEndMode();
			return;
		}

		Validate();
		pDevice->SetVertexShader( m_hVertexShader );
		pDevice->SetPixelShader( NULL );

		static constexpr TVector4 s_SomeVector( 1.0f, 1.0f, 1.0f, 1.0f );
		pDevice->SetVertexShaderConstant( 4, &m_AmbientColour, 1 );
		pDevice->SetVertexShaderConstant( 5, &m_ShadowColour, 1 );
		pDevice->SetVertexShaderConstant( 6, &s_SomeVector, 1 );

		pDevice->SetRenderState( D3DRS_ZWRITEENABLE, 1 );
		pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, 1 );
		pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, 1 );
		pDevice->SetRenderState( D3DRS_ALPHAFUNC, 5 );

		DWORD dwWasSpecularEnabled;
		pDevice->GetRenderState( D3DRS_SPECULARENABLE, &dwWasSpecularEnabled );
		pDevice->SetRenderState( D3DRS_SPECULARENABLE, 0 );
		pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, 4 );
		pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, 2 );
		pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, 0 );
		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, 4 );
		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, 2 );
		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, 0 );
		pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, 1 );
		pDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, 1 );
		pDevice->SetRenderState( D3DRS_CULLMODE, m_bRenderEnvMap ? D3DCULL_CCW : D3DCULL_CW );
	}
}

void AWorldShaderHAL::EndFlush()
{
	auto pRenderInterface = TRenderD3DInterface::Interface();
	auto pDevice          = pRenderInterface->GetDirect3DDevice();

	pDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );
	pDevice->SetPixelShader( NULL );
}

// $Barnyard: FUNCTION 005f5f80
TBOOL AWorldShaderHAL::Create()
{
	m_aOrderTables[ 0 ].Create( this, -3000 );
	m_aOrderTables[ 1 ].Create( this, 100 );
	m_aOrderTables[ 2 ].Create( this, 101 );
	m_aOrderTables[ 3 ].Create( this, 601 );
	m_aOrderTables[ 4 ].Create( this, -400 );
	m_aOrderTables[ 5 ].Create( this, 500 );
	m_aOrderTables[ 6 ].Create( this, -6005 );
	m_aOrderTables[ 7 ].Create( this, -7000 );
	return AWorldShader::Create();
}

static TBOOL CreateWorldVertexShader( const char* a_szFileName, const DWORD* a_pFunction, DWORD* a_pOutVertexShader )
{
	static constexpr DWORD s_ShaderDeclaration[] = {
		0x20000000, 0x40020000, 0x40020001, 0x40020002,
		0x40010003, 0xFFFFFFFF
	};

	return TRenderD3DInterface::CreateVertexShader( s_ShaderDeclaration, a_pFunction, a_pOutVertexShader );
}

// $Barnyard: FUNCTION 005f6b20
TBOOL AWorldShaderHAL::Validate()
{
	if ( IsValidated() )
	{
		return TTRUE;
	}

	// Enable high end mode if it is supported
	SetHighEndMode( TTRUE );

	if ( IsHighEndMode() )
	{
		constexpr static DWORD s_VertexShaderFunction[] = {
			0xFFFE0101, 0x14, 0xC00F0000, 0x90E40000,
			0xA0E40000, 0x2, 0x800F0001, 0xA0E40006,
			0x91E40002, 0x5, 0x800F0000, 0x90E40002,
			0xA0E40004, 0x5, 0x800F0002, 0x80E40001,
			0xA0E40005, 0x2, 0x800F0000, 0x80E40000,
			0x80E40002, 0x1, 0x80080000, 0xA0FF0006,
			0x1, 0xD00F0000, 0x80E40000, 0x5,
			0x80010001, 0x90000002, 0xA0000008, 0x4,
			0xD0080000, 0x80FF0000, 0xA0550008, 0x80000001,
			0x2, 0xE0030000, 0xA0540007, 0x90540003,
			0x2, 0xE0030001, 0xA0FE0007, 0x90540003,
			0xFFFF
		};

		if ( !CreateWorldVertexShader( "AWorldShader_D3D8_Win.vsh", s_VertexShaderFunction, &m_hVertexShader ) )
		{
			TASSERT( !"Couldn't create vertex shader" );
			return TFALSE;
		}
	}

	return AWorldShader::Validate();
}

// $Barnyard: FUNCTION 005f7580
void AWorldShaderHAL::Invalidate()
{
	if ( !IsValidated() ) return;

	if ( !IsHighEndMode() )
	{
		m_SomeList.DeleteAll();
	}
	else if ( m_hVertexShader != 0 )
	{
		TRenderD3DInterface::DestroyVertexShader( m_hVertexShader );
		m_hVertexShader = 0;
	}

	TShader::Invalidate();
}

TBOOL AWorldShaderHAL::TryInvalidate()
{
	Invalidate();
	return TTRUE;
}

TBOOL AWorldShaderHAL::TryValidate()
{
	Validate();
	return TTRUE;
}

// $Barnyard: FUNCTION 005f6cb0
void AWorldShaderHAL::Render( TRenderPacket* a_pRenderPacket )
{
	if ( IsHighEndMode() && a_pRenderPacket && a_pRenderPacket->GetMesh() )
	{
		auto pRenderInterface = TRenderD3DInterface::Interface();
		auto pCurrentContext  = TRenderContextD3D::Upcast( pRenderInterface->GetCurrentContext() );
		auto pDevice          = pRenderInterface->GetDirect3DDevice();

		D3DXMATRIX mMVP;
		D3DXMatrixMultiply( &mMVP, a_pRenderPacket->GetModelViewMatrix(), pCurrentContext->GetProjectionMatrix() );
		D3DXMatrixTranspose( &mMVP, &mMVP );

		pDevice->SetVertexShaderConstant( 0, &mMVP, 4 );

		if ( a_pRenderPacket->GetAlpha() >= 1.0f || isnan( a_pRenderPacket->GetAlpha() ) )
		{
			if ( s_RenderStateFlags & 24 )
			{
				pDevice->SetRenderState( D3DRS_ALPHAREF, m_iAlphaRef );
				s_RenderStateFlags &= ~24;
			}
		}
		else
		{
			pDevice->SetRenderState( D3DRS_ALPHAREF, TUINT8( m_iAlphaRef * a_pRenderPacket->GetAlpha() ) );
			s_RenderStateFlags |= 24;
		}

		auto pMesh     = TSTATICCAST( AWorldMeshHAL, a_pRenderPacket->GetMesh() );
		auto pMaterial = TSTATICCAST( AWorldMaterialHAL, pMesh->GetMaterial() );

		if ( pMaterial->GetBlendMode() != 0 || a_pRenderPacket->GetAlpha() < 1.0f )
		{
			if ( ISZERO( s_RenderStateFlags & 0x1b ) )
			{
				pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
				s_RenderStateFlags |= 0x1b;
			}
		}
		else if ( !ISZERO( s_RenderStateFlags & 0x1b ) )
		{
			pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
			s_RenderStateFlags &= ~0x1b;
		}

		TVector4 vSomeConst;
		vSomeConst.x = 0.0f;
		vSomeConst.y = 1.0f;

		if ( pMesh->IsUnknownState1() )
		{
			vSomeConst.x = 1.0f;
			vSomeConst.y = 0.0f;
		}

		vSomeConst.w = 1.0f;
		vSomeConst.z = 0.0f;

		pDevice->SetVertexShaderConstant( 8, &vSomeConst, 1 );

		// Set vertices
		auto pVertexPool = TSTATICCAST( TVertexPoolResource, pMesh->GetVertexPool() );
		TVALIDPTR( pVertexPool );

		TVertexBlockResource::HALBuffer vertexBuffer;
		pVertexPool->GetHALBuffer( &vertexBuffer );

		pDevice->SetStreamSource( 0, vertexBuffer.apVertexBuffers[ 0 ], sizeof( WorldVertex ) );

		// Set indices
		auto pIndexPool = TSTATICCAST( TIndexPoolResource, pMesh->GetSubMesh( 0 )->pIndexPool );
		TVALIDPTR( pIndexPool );

		TIndexBlockResource::HALBuffer indexBuffer;
		pIndexPool->GetHALBuffer( &indexBuffer );

		pDevice->SetIndices( indexBuffer.pIndexBuffer, vertexBuffer.uiVertexOffset );

		// Draw mesh
		pDevice->DrawIndexedPrimitive(
		    D3DPT_TRIANGLESTRIP,
		    0,
		    pVertexPool->GetNumVertices(),
		    indexBuffer.uiIndexOffset,
		    pIndexPool->GetNumIndices() - 2
		);
	}
}

// $Barnyard: FUNCTION 005f6500
void AWorldShaderHAL::EnableRenderEnvMap( TBOOL a_bEnable )
{
	m_bRenderEnvMap = a_bEnable;
}

// $Barnyard: FUNCTION 005f7760
TBOOL AWorldShaderHAL::IsAlphaBlendMaterial()
{
	return m_bAlphaBlendMaterial;
}

// $Barnyard: FUNCTION 005f7780
void AWorldShaderHAL::SetAlphaBlendMaterial( TBOOL a_bIsAlphaBlendMaterial )
{
	m_bAlphaBlendMaterial = a_bIsAlphaBlendMaterial;
}

// $Barnyard: FUNCTION 005f6c00
AWorldMaterial* AWorldShaderHAL::CreateMaterial( const TCHAR* a_szName )
{
	Validate();

	auto pMaterial = new AWorldMaterialHAL();
	pMaterial->SetShader( this );

	if ( AWorldShaderHAL::IsAlphaBlendMaterial() )
	{
		auto pAlphaBlendMaterial = new AWorldMaterialHAL();
		pAlphaBlendMaterial->SetShader( this );
		pAlphaBlendMaterial->Create( 1 );

		pMaterial->SetAlphaBlendMaterial( pAlphaBlendMaterial );
	}

	return pMaterial;
}

// $Barnyard: FUNCTION 005f6b80
AWorldMesh* AWorldShaderHAL::CreateMesh( const TCHAR* a_szName )
{
	Validate();

	auto pMesh = new AWorldMeshHAL();
	pMesh->SetOwnerShader( this );

	return pMesh;
}

// $Barnyard: FUNCTION 005f7790
TBOOL AWorldShaderHAL::IsHighEndMode()
{
	return m_bIsHighEndMode;
}

// $Barnyard: FUNCTION 005f6010
void AWorldShaderHAL::SetHighEndMode( TBOOL a_bEnable )
{
	m_bIsHighEndMode = a_bEnable && IsCapableShaders();
}

TBOOL AWorldShaderHAL::IsCapableShaders()
{
	D3DCAPS8 caps;

	auto pRenderInterface = TRenderD3DInterface::Interface();
	auto pDevice          = pRenderInterface->GetDirect3DDevice();

	if ( !pDevice )
	{
		auto pDirect3D = pRenderInterface->GetDirect3D();

		TVALIDPTR( pDirect3D );
		if ( !pDirect3D ) return TFALSE;

		pDirect3D->GetDeviceCaps( 0, D3DDEVTYPE_HAL, &caps );
	}
	else
	{
		pDevice->GetDeviceCaps( &caps );
	}

	return ( ( caps.VertexShaderVersion & 0xFFFF ) > 256 ) && ( caps.DevCaps & 0x10000 );
}

TBOOL AWorldShaderHAL::IsRenderEnvMapEnabled()
{
	return m_bRenderEnvMap;
}

void* AWorldShaderHAL::CreateUnknown( void*, void*, void*, void* )
{
	TASSERT( !"What's this?" );
	return TNULL;
}

// $Barnyard: FUNCTION 005f6440
void AWorldShaderHAL::SetColours( const Toshi::TVector4& a_rShadowColour, const Toshi::TVector4 a_rAmbientColour )
{
	m_ShadowColour  = a_rShadowColour;
	m_AmbientColour = a_rAmbientColour;

	Toshi::TMath::Clip( m_ShadowColour.x, 0.0f, 1.0f );
	Toshi::TMath::Clip( m_ShadowColour.y, 0.0f, 1.0f );
	Toshi::TMath::Clip( m_ShadowColour.z, 0.0f, 1.0f );
	Toshi::TMath::Clip( m_AmbientColour.x, 0.0f, 1.0f );
	Toshi::TMath::Clip( m_AmbientColour.y, 0.0f, 1.0f );
	Toshi::TMath::Clip( m_AmbientColour.z, 0.0f, 1.0f );
}

// $Barnyard: FUNCTION 005f6270
void AWorldShaderHAL::SetupLowEndMode()
{
	auto pRenderInterface = TRenderD3DInterface::Interface();
	auto pCurrentContext  = TRenderContextD3D::Upcast( pRenderInterface->GetCurrentContext() );
	auto pDevice          = pRenderInterface->GetDirect3DDevice();

	pDevice->SetVertexShader( D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_NORMAL | D3DFVF_XYZ );
	pDevice->SetPixelShader( NULL );
	pDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
	pDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_SRCBLEND, 5 );
	pDevice->SetRenderState( D3DRS_DESTBLEND, 6 );
	pDevice->SetRenderState( D3DRS_ALPHAFUNC, 5 );

	DWORD dwWasSpecularEnabled;
	pDevice->GetRenderState( D3DRS_SPECULARENABLE, &dwWasSpecularEnabled );
	pDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );

	pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, 4 );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, 2 );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, 0 );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, 4 );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, 2 );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, 0 );
	pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, 1 );
	pDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, 1 );

	pDevice->SetRenderState( D3DRS_CULLMODE, m_bRenderEnvMap ? D3DCULL_CCW : D3DCULL_CW );

	static TMatrix44 s_IdentityMatrix = TMatrix44::IDENTITY;

	pDevice->SetTransform( D3DTS_VIEW, s_IdentityMatrix );
	pDevice->SetTransform( D3DTS_PROJECTION, pCurrentContext->GetProjectionMatrix() );
	pDevice->SetRenderState( D3DRS_SPECULARENABLE, dwWasSpecularEnabled );
	pDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );
}
