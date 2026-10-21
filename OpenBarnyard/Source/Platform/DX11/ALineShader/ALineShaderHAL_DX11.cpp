#include "pch.h"
#include "ALineShaderHAL_DX11.h"

#include <Platform/DX11/TRenderInterface_DX11.h>
#include <Platform/DX11/TRenderContext_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( ALineMaterialHAL );

TDEFINE_CLASS( ALineMeshHAL );

ALineMaterialHAL::~ALineMaterialHAL()
{
}

ALineMeshHAL::~ALineMeshHAL()
{
}

TDEFINE_CLASS( ALineShaderHAL );

// The fixed function pipeline for D3DFVF_XYZ | D3DFVF_DIFFUSE vertices, stage 0 modulates with no texture
static const TCHAR s_szLineShader[] = R"(
cbuffer Constants : register( b0 )
{
	float4x4 g_mModelViewProjection : packoffset( c0 );
	float4   g_vAlphaTest           : packoffset( c4 );
};

struct VS_IN
{
	float3 vPosition : POSITION;
	float4 vColour : COLOR;
};

struct PS_IN
{
	float4 vPosition : SV_POSITION;
	float4 vColour : COLOR;
};

PS_IN vs_main( VS_IN In )
{
	PS_IN Out;
	Out.vPosition = mul( float4( In.vPosition, 1.0f ), g_mModelViewProjection );
	Out.vColour   = In.vColour;
	return Out;
}

float4 ps_main( PS_IN In ) : SV_TARGET
{
	AlphaTest( In.vColour.a, g_vAlphaTest );
	return In.vColour;
}
)";

ALineShaderHAL::ALineShaderHAL()
{
	m_pLineVertexBuffer     = TNULL;
	m_pTriangleVertexBuffer = TNULL;
	m_pVertexShader         = TNULL;
	m_pPixelShader          = TNULL;
	m_pInputLayout          = TNULL;
	m_uiNumLineVertices     = 0;
	m_uiNumTriangleVertices = 0;
}

ALineShaderHAL::~ALineShaderHAL()
{
}

void ALineShaderHAL::OnDestroy()
{
	Invalidate();
	TShader::OnDestroy();
}

TBOOL ALineShaderHAL::Create()
{
	return ALineShader::Create();
}

TBOOL ALineShaderHAL::Validate()
{
	auto pRender = TRenderD3DInterface::Interface();

	if ( !m_pLineVertexBuffer )
	{
		m_pLineVertexBuffer = pRender->CreateBuffer( D3D11_BIND_VERTEX_BUFFER, sizeof( m_aLineVertices ), TNULL, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE );
		if ( !m_pLineVertexBuffer ) return TFALSE;

		m_uiNumLineVertices = 0;
	}

	if ( !m_pTriangleVertexBuffer )
	{
		m_pTriangleVertexBuffer = pRender->CreateBuffer( D3D11_BIND_VERTEX_BUFFER, sizeof( m_aTriangleVertices ), TNULL, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE );
		if ( !m_pTriangleVertexBuffer ) return TFALSE;

		m_uiNumTriangleVertices = 0;
	}

	if ( !m_pVertexShader )
	{
		ID3DBlob* pVSBlob = TRenderD3DInterface::CompileShader( s_szLineShader, "vs_main", "vs_4_0", TNULL );
		ID3DBlob* pPSBlob = TRenderD3DInterface::CompileShader( s_szLineShader, "ps_main", "ps_4_0", TNULL );

		if ( !pVSBlob || !pPSBlob ||
		     !TRenderD3DInterface::CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexShader ) ||
		     !TRenderD3DInterface::CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), &m_pPixelShader ) )
		{
			TASSERT( !"Couldn't create the shaders" );
			return TFALSE;
		}

		static constexpr D3D11_INPUT_ELEMENT_DESC s_aInputLayout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		pRender->GetDevice()->CreateInputLayout( s_aInputLayout, TARRAYSIZE( s_aInputLayout ), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pInputLayout );

		pVSBlob->Release();
		pPSBlob->Release();
	}

	return TShader::Validate();
}

void ALineShaderHAL::Invalidate()
{
	ID3D11DeviceChild** appObjects[] = {
		TREINTERPRETCAST( ID3D11DeviceChild**, &m_pLineVertexBuffer ),
		TREINTERPRETCAST( ID3D11DeviceChild**, &m_pTriangleVertexBuffer ),
		TREINTERPRETCAST( ID3D11DeviceChild**, &m_pVertexShader ),
		TREINTERPRETCAST( ID3D11DeviceChild**, &m_pPixelShader ),
		TREINTERPRETCAST( ID3D11DeviceChild**, &m_pInputLayout ),
	};

	for ( auto ppObject : appObjects )
	{
		if ( *ppObject )
		{
			( *ppObject )->Release();
			*ppObject = TNULL;
		}
	}

	TShader::Invalidate();
}

TBOOL ALineShaderHAL::TryInvalidate()
{
	Invalidate();
	return TTRUE;
}

TBOOL ALineShaderHAL::TryValidate()
{
	Validate();
	return TTRUE;
}

void ALineShaderHAL::DrawVertices( ID3D11Buffer* a_pVertexBuffer, D3D11_PRIMITIVE_TOPOLOGY a_eTopology, const Vertex* a_pVertices, TUINT a_uiNumVertices )
{
	auto pRender = TRenderD3DInterface::Interface();

	D3D11_MAPPED_SUBRESOURCE oMapped;
	if ( SUCCEEDED( pRender->GetDeviceContext()->Map( a_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &oMapped ) ) )
	{
		TUtil::MemCopy( oMapped.pData, a_pVertices, a_uiNumVertices * sizeof( Vertex ) );
		pRender->GetDeviceContext()->Unmap( a_pVertexBuffer, 0 );
	}

	const TVector4 vAlphaTest = pRender->GetAlphaTestParams();
	pRender->SetVec4InVSBuffer( 4, &vAlphaTest );

	pRender->DrawNonIndexed( a_eTopology, a_pVertexBuffer, a_uiNumVertices, sizeof( Vertex ), 0, 0 );
}

void ALineShaderHAL::FlushLines()
{
	Validate();

	if ( !m_pLineVertexBuffer && !m_pTriangleVertexBuffer ) return;
	if ( m_uiNumLineVertices == 0 && m_uiNumTriangleVertices == 0 ) return;

	TRenderD3DInterface* pRender = TRenderD3DInterface::Interface();
#ifdef BARNYARD_COMMUNITY_PATCH
	TRenderContextD3D* pContext = TRenderContextD3D::Upcast( pRender->GetCurrentContext() );
#else
	TRenderContextD3D* pContext = TRenderContextD3D::Upcast( pRender->GetCreatedContext() );
#endif

	// World is the model view matrix and view is the identity
	TMatrix44 oModelViewProjection;
	oModelViewProjection.Multiply( pContext->GetProjectionMatrix(), pContext->GetModelViewMatrix() );

	TMatrix44 oModelViewProjectionTransposed;
	oModelViewProjectionTransposed.Transpose( oModelViewProjection );
	pRender->SetVec4InVSBuffer( 0, &oModelViewProjectionTransposed, 4 );

	for ( UINT i = 0; i < 5; i++ )
		pRender->SetTexture( i, TNULL );

	pRender->SetShaders( m_pVertexShader, m_pPixelShader, m_pInputLayout );

	if ( m_pLineVertexBuffer && m_uiNumLineVertices != 0 )
	{
		DrawVertices( m_pLineVertexBuffer, D3D11_PRIMITIVE_TOPOLOGY_LINELIST, m_aLineVertices, m_uiNumLineVertices / 2 * 2 );
		m_uiNumLineVertices = 0;
	}

	if ( m_pTriangleVertexBuffer && m_uiNumTriangleVertices != 0 )
	{
		pRender->SetAlphaFunc( D3D11_COMPARISON_ALWAYS );
		pRender->SetBlendOp( D3D11_BLEND_OP_ADD );
		pRender->SetAlphaBlendEnable( TTRUE );
		pRender->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
		pRender->SetDestBlend( D3D11_BLEND_INV_SRC_ALPHA );

		D3D11_CULL_MODE eCullMode = pRender->GetCullMode();
		pRender->SetCullMode( D3D11_CULL_NONE );

		DrawVertices( m_pTriangleVertexBuffer, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_aTriangleVertices, m_uiNumTriangleVertices / 3 * 3 );
		m_uiNumTriangleVertices = 0;

		pRender->SetCullMode( eCullMode );
	}
}

void ALineShaderHAL::AddLineDirect( const TVector3& a_rcStart, const TVector3& a_rcEnd, TUINT32 a_uiStartColour, TUINT32 a_uiEndColour )
{
	// NOTE: lines that don't fit are dropped
	if ( m_uiNumLineVertices < MAX_NUM_VERTICES )
	{
		m_aLineVertices[ m_uiNumLineVertices ].uiColour        = a_uiStartColour;
		m_aLineVertices[ m_uiNumLineVertices ].vecPosition     = a_rcStart;
		m_aLineVertices[ m_uiNumLineVertices + 1 ].uiColour    = a_uiEndColour;
		m_aLineVertices[ m_uiNumLineVertices + 1 ].vecPosition = a_rcEnd;
		m_uiNumLineVertices += 2;
	}
}

void ALineShaderHAL::AddTriangle( const TVector3 a_aVertices[ 3 ], TUINT32 a_uiColour )
{
#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: the original only flushes when the array is full, 40000 isn't a multiple of 3, so the
	// 13334th triangle was written past the end of the array
	if ( m_uiNumTriangleVertices + 3 > MAX_NUM_VERTICES )
#else
	if ( m_uiNumTriangleVertices >= MAX_NUM_VERTICES )
#endif // BARNYARD_COMMUNITY_PATCH
		FlushLines();

	m_aTriangleVertices[ m_uiNumTriangleVertices ].uiColour        = a_uiColour;
	m_aTriangleVertices[ m_uiNumTriangleVertices + 1 ].uiColour    = a_uiColour;
	m_aTriangleVertices[ m_uiNumTriangleVertices + 2 ].uiColour    = a_uiColour;
	m_aTriangleVertices[ m_uiNumTriangleVertices ].vecPosition     = a_aVertices[ 0 ];
	m_aTriangleVertices[ m_uiNumTriangleVertices + 1 ].vecPosition = a_aVertices[ 1 ];
	m_aTriangleVertices[ m_uiNumTriangleVertices + 2 ].vecPosition = a_aVertices[ 2 ];
	m_uiNumTriangleVertices += 3;
}
