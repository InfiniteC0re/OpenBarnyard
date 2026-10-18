#include "pch.h"
#include "ALineShaderHAL_DX8.h"

#include <Platform/DX8/TRenderInterface_DX8.h>
#include <Platform/DX8/TRenderContext_DX8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005f53c0, CreateObject 005f59a0, CreateObjectInPlace 005f53d0
TDEFINE_CLASS( ALineMaterialHAL );

// NOTE: GetClass 005f5420, CreateObject 005f5a40, CreateObjectInPlace 005f5430
TDEFINE_CLASS( ALineMeshHAL );

// $Barnyard: FUNCTION 005f5a30
// $Barnyard: FUNCTION 005f5a10
ALineMaterialHAL::~ALineMaterialHAL()
{
}

// $Barnyard: FUNCTION 005f5ad0
// $Barnyard: FUNCTION 005f5ab0
ALineMeshHAL::~ALineMeshHAL()
{
}

// NOTE: GetClass 005f53b0, CreateObject 005f5b30, CreateObjectInPlace 005f5bb0
TDEFINE_CLASS( ALineShaderHAL );

// $Barnyard: FUNCTION 005f5ae0
ALineShaderHAL::ALineShaderHAL()
{
	m_pLineVertexBuffer     = TNULL;
	m_pTriangleVertexBuffer = TNULL;
	m_uiNumLineVertices     = 0;
	m_uiNumTriangleVertices = 0;
}

// $Barnyard: FUNCTION 005f5b20
// $Barnyard: FUNCTION 005f5b00
ALineShaderHAL::~ALineShaderHAL()
{
}

// $Barnyard: FUNCTION 005f5490
void ALineShaderHAL::OnDestroy()
{
	if ( m_pLineVertexBuffer )
	{
		m_pLineVertexBuffer->Release();
		m_pLineVertexBuffer = TNULL;
	}

	if ( m_pTriangleVertexBuffer )
	{
		m_pTriangleVertexBuffer->Release();
		m_pTriangleVertexBuffer = TNULL;
	}

	TShader::OnDestroy();
}

// $Barnyard: FUNCTION 005f5480
TBOOL ALineShaderHAL::Create()
{
	return ALineShader::Create();
}

// $Barnyard: FUNCTION 005f54d0
TBOOL ALineShaderHAL::Validate()
{
	IDirect3DDevice8* pDevice = TRenderD3DInterface::Interface()->GetDirect3DDevice();

	if ( !m_pLineVertexBuffer )
	{
		if ( FAILED( pDevice->CreateVertexBuffer( sizeof( m_aLineVertices ), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &m_pLineVertexBuffer ) ) )
			return TFALSE;

		m_uiNumLineVertices = 0;
	}

	if ( !m_pTriangleVertexBuffer )
	{
		if ( FAILED( pDevice->CreateVertexBuffer( sizeof( m_aTriangleVertices ), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &m_pTriangleVertexBuffer ) ) )
			return TFALSE;

		m_uiNumTriangleVertices = 0;
	}

	return TShader::Validate();
}

// $Barnyard: FUNCTION 005f5550
void ALineShaderHAL::Invalidate()
{
	if ( m_pLineVertexBuffer )
	{
		m_pLineVertexBuffer->Release();
		m_pLineVertexBuffer = TNULL;
	}

	if ( m_pTriangleVertexBuffer )
	{
		m_pTriangleVertexBuffer->Release();
		m_pTriangleVertexBuffer = TNULL;
	}

	TShader::Invalidate();
}

// NOTE: shared with ASysShaderHAL::TryInvalidate (005f3690)
TBOOL ALineShaderHAL::TryInvalidate()
{
	Invalidate();
	return TTRUE;
}

// NOTE: shared with ASysShaderHAL::TryValidate (005f36a0)
TBOOL ALineShaderHAL::TryValidate()
{
	Validate();
	return TTRUE;
}

// $Barnyard: FUNCTION 005f56b0
void ALineShaderHAL::FlushLines()
{
	Validate();

	if ( !m_pLineVertexBuffer && !m_pTriangleVertexBuffer ) return;
	if ( m_uiNumLineVertices == 0 && m_uiNumTriangleVertices == 0 ) return;

	TRenderD3DInterface* pRender  = TRenderD3DInterface::Interface();
	IDirect3DDevice8*    pDevice  = pRender->GetDirect3DDevice();
#ifdef BARNYARD_COMMUNITY_PATCH
	TRenderContextD3D* pContext = TRenderContextD3D::Upcast( pRender->GetCurrentContext() );
#else
	TRenderContextD3D* pContext = TRenderContextD3D::Upcast( pRender->GetCreatedContext() );
#endif

	TMatrix44 oViewMatrix = TMatrix44::IDENTITY;
	pDevice->SetTransform( D3DTS_VIEW, TREINTERPRETCAST( const D3DMATRIX*, &oViewMatrix ) );
	pDevice->SetTransform( D3DTS_PROJECTION, TREINTERPRETCAST( const D3DMATRIX*, &pContext->GetProjectionMatrix() ) );
	pDevice->SetTransform( D3DTS_WORLD, TREINTERPRETCAST( const D3DMATRIX*, &pContext->GetModelViewMatrix() ) );

	pDevice->SetPixelShader( 0 );
	pDevice->SetTexture( 0, TNULL );
	pDevice->SetTexture( 1, TNULL );
	pDevice->SetTexture( 2, TNULL );
	pDevice->SetTexture( 3, TNULL );
	pDevice->SetTexture( 4, TNULL );
	pDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
	pDevice->SetTextureStageState( 1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
	pDevice->SetTextureStageState( 2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
	pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_CURRENT );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	pDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
	pDevice->SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE );
	pDevice->SetIndices( TNULL, 0 );

	if ( m_pLineVertexBuffer && m_uiNumLineVertices != 0 )
	{
		BYTE* pVertices;
		if ( SUCCEEDED( m_pLineVertexBuffer->Lock( 0, m_uiNumLineVertices * sizeof( Vertex ), &pVertices, D3DLOCK_DISCARD ) ) )
		{
			TUtil::MemCopy( pVertices, m_aLineVertices, m_uiNumLineVertices * sizeof( Vertex ) );
			m_pLineVertexBuffer->Unlock();
		}

		pDevice->SetStreamSource( 0, m_pLineVertexBuffer, sizeof( Vertex ) );
		pDevice->DrawPrimitive( D3DPT_LINELIST, 0, m_uiNumLineVertices / 2 );
		m_uiNumLineVertices = 0;
	}

	if ( m_pTriangleVertexBuffer && m_uiNumTriangleVertices != 0 )
	{
		pDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_ALWAYS );
		pDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
		pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

		DWORD uiCullMode;
		pDevice->GetRenderState( D3DRS_CULLMODE, &uiCullMode );
		pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

		BYTE* pVertices;
		if ( SUCCEEDED( m_pTriangleVertexBuffer->Lock( 0, m_uiNumTriangleVertices * sizeof( Vertex ), &pVertices, D3DLOCK_DISCARD ) ) )
		{
			TUtil::MemCopy( pVertices, m_aTriangleVertices, m_uiNumTriangleVertices * sizeof( Vertex ) );
			m_pTriangleVertexBuffer->Unlock();
		}

		pDevice->SetStreamSource( 0, m_pTriangleVertexBuffer, sizeof( Vertex ) );
		pDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, m_uiNumTriangleVertices / 3 );
		m_uiNumTriangleVertices = 0;

		pDevice->SetRenderState( D3DRS_CULLMODE, uiCullMode );
	}
}

// $Barnyard: FUNCTION 005f5590
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

// $Barnyard: FUNCTION 005f5600
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
