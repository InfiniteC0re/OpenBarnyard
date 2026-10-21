#include "pch.h"
#include "AGUI2Renderer_DX11.h"

#include "Assets/AMaterialLibraryManager.h"
#include "GUI/AGUI2.h"
#include "GUI/AGUI2Material.h"

#include "Platform/DX11/TTextureResourceHAL_DX11.h"
#include "Platform/DX11/TRenderInterface_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AGUI2RendererDX8::AGUI2RendererDX8()
{
	m_pTransforms       = new T2GUITransform[ MAX_NUM_TRANSFORMS ];
	m_iTransformCount   = 0;
	m_bIsTransformDirty = TFALSE;
}

AGUI2RendererDX8::~AGUI2RendererDX8()
{
	delete[] m_pTransforms;
}

T2GUIMaterial* AGUI2RendererDX8::CreateMaterial( const TCHAR* a_szTextureName )
{
	return CreateMaterial(
	    GetTexture( a_szTextureName )
	);
}

T2GUIMaterial* AGUI2RendererDX8::CreateMaterial( Toshi::TTexture* a_pTexture )
{
	auto pMaterial = new AGUI2Material;

	pMaterial->Create();
	pMaterial->m_pTextureResource = a_pTexture;

	return pMaterial;
}

void AGUI2RendererDX8::DestroyMaterial( T2GUIMaterial* a_pMaterial )
{
	if ( a_pMaterial )
	{
		TSTATICCAST( AGUI2Material, a_pMaterial )->OnDestroy();
		delete a_pMaterial;
	}
}

Toshi::TTexture* AGUI2RendererDX8::GetTexture( const TCHAR* a_szTextureName )
{
	return AMaterialLibraryManager::GetSingleton()->FindTexture( a_szTextureName );
}

TUINT AGUI2RendererDX8::GetWidth( T2GUIMaterial* a_pMaterial )
{
	return TSTATICCAST( AGUI2Material, a_pMaterial )->m_pTextureResource->GetWidth();
}

TUINT AGUI2RendererDX8::GetHeight( T2GUIMaterial* a_pMaterial )
{
	return TSTATICCAST( AGUI2Material, a_pMaterial )->m_pTextureResource->GetHeight();
}

void AGUI2RendererDX8::BeginScene()
{
	auto pRender = TRenderD3DInterface::Interface();

	TRenderD3DInterface::FlushShaders();
	auto pDisplayParams = pRender->GetCurrentDisplayParams();

	TFLOAT fRootWidth;
	TFLOAT fRootHeight;
	AGUI2::GetContext()->GetRootElement()->GetDimensions( fRootWidth, fRootHeight );

	auto& rTransform              = m_pTransforms[ m_iTransformCount ];
	rTransform.m_aMatrixRows[ 0 ] = { pDisplayParams->uiWidth / fRootWidth, 0.0f };
	rTransform.m_aMatrixRows[ 1 ] = { 0.0f, -TFLOAT( pDisplayParams->uiHeight ) / fRootHeight };
	rTransform.m_vecTranslation   = { 0.0f, 0.0f };

	pRender->SetTransform( TRenderD3DInterface::TRANSFORM_WORLD, TMatrix44::IDENTITY );
	pRender->SetTransform( TRenderD3DInterface::TRANSFORM_VIEW, TMatrix44::IDENTITY );

	TMatrix44 projection = {
		2.0f / (TFLOAT)pDisplayParams->uiWidth, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f / (TFLOAT)pDisplayParams->uiHeight, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	pRender->SetTransform( TRenderD3DInterface::TRANSFORM_PROJECTION, projection );
	pRender->SetFixedFunctionShader();
	pRender->SetAlphaTestEnable( TTRUE );
	pRender->SetZFunc( D3D11_COMPARISON_ALWAYS );
	pRender->SetZWriteEnable( TFALSE );
	pRender->SetCullMode( D3D11_CULL_NONE );

	SetColour( 0xFFFFFFFF );
	m_pMaterial = TNULL;

	pRender->SetAlphaBlendEnable( TTRUE );
	pRender->SetAlphaFunc( D3D11_COMPARISON_GREATER );
	pRender->SetAlphaRef( 0 );
	pRender->SetBlendOp( D3D11_BLEND_OP_ADD );
	pRender->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
	pRender->SetDestBlend( D3D11_BLEND_INV_SRC_ALPHA );
	pRender->SetDepthBias( 0, 0.0f );
	pRender->SetTextureOp( TRenderD3DInterface::TEXTUREOP_SELECTDIFFUSE );
	sm_bUnknownFlag = TFALSE;
}

void AGUI2RendererDX8::EndScene()
{
	auto pRender = TRenderD3DInterface::Interface();

	pRender->SetZWriteEnable( TTRUE );
	pRender->SetZFunc( D3D11_COMPARISON_LESS_EQUAL );
	pRender->SetDepthBias( 0, 0.0f );
	pRender->SetAlphaBlendEnable( TTRUE );
	pRender->SetBlendOp( D3D11_BLEND_OP_ADD );
	pRender->SetAlphaTestEnable( TFALSE );
	pRender->SetAlphaRef( 0 );
	pRender->SetAlphaFunc( D3D11_COMPARISON_GREATER );
	pRender->SetTextureOp( TRenderD3DInterface::TEXTUREOP_SELECTDIFFUSE );
	pRender->SetTexture( 1, TNULL );
	pRender->SetTextureAddress( 0, ADDRESSINGMODE_WRAP, TEXCOORD_UV );
}

void AGUI2RendererDX8::ResetRenderer()
{
	auto pRender = TRenderD3DInterface::Interface();

	pRender->SetBlendOp( D3D11_BLEND_OP_ADD );
	pRender->SetZWriteEnable( TTRUE );
	pRender->SetZFunc( D3D11_COMPARISON_LESS_EQUAL );
	pRender->SetDepthBias( 0, 0.0f );
	pRender->SetAlphaBlendEnable( TTRUE );
	pRender->SetAlphaTestEnable( TFALSE );
	pRender->SetAlphaRef( 0 );
	pRender->SetAlphaFunc( D3D11_COMPARISON_GREATER );
	pRender->SetTextureOp( TRenderD3DInterface::TEXTUREOP_SELECTDIFFUSE );
	pRender->SetTexture( 1, TNULL );
}

void AGUI2RendererDX8::PrepareRenderer()
{
	auto pRender        = TRenderD3DInterface::Interface();
	auto pDisplayParams = pRender->GetCurrentDisplayParams();

	pRender->SetTransform( TRenderD3DInterface::TRANSFORM_WORLD, TMatrix44::IDENTITY );
	pRender->SetTransform( TRenderD3DInterface::TRANSFORM_VIEW, TMatrix44::IDENTITY );

	TMatrix44 projection = {
		2.0f / (TFLOAT)pDisplayParams->uiWidth, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f / (TFLOAT)pDisplayParams->uiHeight, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	pRender->SetTransform( TRenderD3DInterface::TRANSFORM_PROJECTION, projection );
	pRender->SetFixedFunctionShader();
	pRender->SetAlphaTestEnable( TTRUE );
	pRender->SetZFunc( D3D11_COMPARISON_ALWAYS );
	pRender->SetZWriteEnable( TFALSE );
	pRender->SetCullMode( D3D11_CULL_NONE );

	sm_fZCoordinate = 0.1f;

	// Force material to update
	auto pMaterial = m_pMaterial;
	m_pMaterial    = (AGUI2Material*)( ~(uintptr_t)pMaterial );
	SetMaterial( pMaterial );

	// Force colour to update
	auto uiColour = m_uiColour;
	m_uiColour    = ~uiColour;
	SetColour( uiColour );

	m_bIsTransformDirty = TTRUE;
}

void AGUI2RendererDX8::SetMaterial( T2GUIMaterial* a_pMaterial )
{
	auto pRender = TRenderD3DInterface::Interface();

	if ( a_pMaterial == m_pMaterial )
	{
		return;
	}

	pRender->SetAlphaTestEnable( TFALSE );
	pRender->SetCullMode( D3D11_CULL_NONE );
	sm_fZCoordinate = 0.1f;

	if ( a_pMaterial == TNULL )
	{
		pRender->SetAlphaBlendEnable( TTRUE );
		pRender->SetAlphaFunc( D3D11_COMPARISON_GREATER );
		pRender->SetAlphaRef( 0 );
		pRender->SetBlendOp( D3D11_BLEND_OP_ADD );
		pRender->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
		pRender->SetDestBlend( D3D11_BLEND_INV_SRC_ALPHA );
		pRender->SetTextureOp( TRenderD3DInterface::TEXTUREOP_SELECTDIFFUSE );
		m_pMaterial = TNULL;
	}
	else
	{
		auto pMaterial = TSTATICCAST( AGUI2Material, a_pMaterial );
		auto pTexture  = TSTATICCAST( Toshi::TTextureResourceHAL, pMaterial->m_pTextureResource );
		pTexture->Validate();

		pRender->SetTexture( 0, pTexture->GetD3DTexture() );

		switch ( pMaterial->m_eBlendState )
		{
			case T2GUIMaterial::BLENDSTATE_OPAQUE:
				pRender->SetAlphaBlendEnable( TFALSE );
				pRender->SetAlphaFunc( D3D11_COMPARISON_GREATER );
				pRender->SetAlphaRef( 0 );
				break;
			case T2GUIMaterial::BLENDSTATE_ALPHA:
				pRender->SetAlphaBlendEnable( TTRUE );
				pRender->SetAlphaFunc( D3D11_COMPARISON_GREATER );
				pRender->SetAlphaRef( 0 );
				pRender->SetBlendOp( D3D11_BLEND_OP_ADD );
				pRender->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
				pRender->SetDestBlend( D3D11_BLEND_INV_SRC_ALPHA );
				pRender->SetZWriteEnable( TFALSE );
				break;
			case T2GUIMaterial::BLENDSTATE_ADDITIVE:
				pRender->SetAlphaBlendEnable( TTRUE );
				pRender->SetAlphaFunc( D3D11_COMPARISON_GREATER );
				pRender->SetAlphaRef( 0 );
				pRender->SetBlendOp( D3D11_BLEND_OP_ADD );
				pRender->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
				pRender->SetDestBlend( D3D11_BLEND_ONE );
				pRender->SetZWriteEnable( TFALSE );
				break;
			case T2GUIMaterial::BLENDSTATE_SUBTRACTIVE:
				pRender->SetAlphaBlendEnable( TTRUE );
				pRender->SetAlphaFunc( D3D11_COMPARISON_GREATER );
				pRender->SetAlphaRef( 0 );
				pRender->SetBlendOp( D3D11_BLEND_OP_REV_SUBTRACT );
				pRender->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
				pRender->SetDestBlend( D3D11_BLEND_ONE );
				pRender->SetZWriteEnable( TFALSE );
				break;
			case T2GUIMaterial::BLENDSTATE_MASK:
				pRender->SetSrcBlend( D3D11_BLEND_ZERO );
				pRender->SetDestBlend( D3D11_BLEND_ONE );
				pRender->SetAlphaBlendEnable( TTRUE );
				pRender->SetBlendOp( D3D11_BLEND_OP_ADD );
				pRender->SetZEnable( TTRUE );
				pRender->SetZWriteEnable( TTRUE );
				pRender->SetZFunc( D3D11_COMPARISON_LESS_EQUAL );
				pRender->SetAlphaRef( 0x3c );
				pRender->SetAlphaTestEnable( TTRUE );
				pRender->SetAlphaFunc( D3D11_COMPARISON_GREATER_EQUAL );

				if ( !sm_bUnknownFlag )
				{
					sm_fZCoordinate = 0.05f;
				}
				else
				{
					sm_fZCoordinate = 0.02f;
				}

				break;
			case T2GUIMaterial::BLENDSTATE_MASKED:
				pRender->SetZEnable( TTRUE );
				pRender->SetZWriteEnable( TFALSE );
				pRender->SetZFunc( D3D11_COMPARISON_LESS_EQUAL );
				pRender->SetAlphaTestEnable( TFALSE );
				pRender->SetAlphaFunc( D3D11_COMPARISON_GREATER );
				pRender->SetAlphaRef( 0 );
				pRender->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
				pRender->SetDestBlend( D3D11_BLEND_INV_SRC_ALPHA );
				pRender->SetAlphaBlendEnable( TTRUE );

				if ( sm_bUnknownFlag )
				{
					sm_fZCoordinate = 0.03f;
					sm_bUnknownFlag = TFALSE;
				}

				break;
			case T2GUIMaterial::BLENDSTATE_ALPHA_ZWRITE:
				pRender->SetAlphaTestEnable( TFALSE );
				pRender->SetAlphaFunc( D3D11_COMPARISON_GREATER );
				pRender->SetAlphaRef( 0 );
				pRender->SetAlphaBlendEnable( TTRUE );
				pRender->SetBlendOp( D3D11_BLEND_OP_ADD );
				pRender->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
				pRender->SetDestBlend( D3D11_BLEND_INV_SRC_ALPHA );
				pRender->SetZWriteEnable( TTRUE );
				pRender->SetZFunc( D3D11_COMPARISON_LESS_EQUAL );
				sm_fZCoordinate = 0.04f;
				sm_bUnknownFlag = TTRUE;
				break;
			default:
				pRender->SetAlphaBlendEnable( TTRUE );
				pRender->SetAlphaFunc( D3D11_COMPARISON_GREATER );
				pRender->SetAlphaRef( 0 );
				pRender->SetBlendOp( D3D11_BLEND_OP_ADD );
				pRender->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
				pRender->SetDestBlend( D3D11_BLEND_INV_SRC_ALPHA );
				break;
		}

		if ( pMaterial->m_eTextureAddress == T2GUIMaterial::TEXTUREADDRESS_CLAMP )
		{
			pRender->SetTextureAddress( 0, ADDRESSINGMODE_CLAMP );
		}
		else if ( pMaterial->m_eTextureAddress == T2GUIMaterial::TEXTUREADDRESS_MIRROR )
		{
			pRender->SetTextureAddress( 0, ADDRESSINGMODE_MIRROR );
		}
		else
		{
			pRender->SetTextureAddress( 0, ADDRESSINGMODE_WRAP );
		}

		// TODO: mip map LOD bias of the material
		pRender->SetTextureMinFilter( 0, TRenderD3DInterface::TEXTUREFILTER_LINEAR );
		pRender->SetTextureMagFilter( 0, TRenderD3DInterface::TEXTUREFILTER_LINEAR );
		pRender->SetTextureMipFilter( 0, TRenderD3DInterface::TEXTUREFILTER_POINT );
		pRender->SetTextureOp( TRenderD3DInterface::TEXTUREOP_MODULATE );
		m_pMaterial = pMaterial;
	}
}

void AGUI2RendererDX8::PushTransform( const T2GUITransform& a_rTransform, const Toshi::TVector2& a_rVec1, const Toshi::TVector2& a_rVec2 )
{
	TASSERT( m_iTransformCount < MAX_NUM_TRANSFORMS );
	auto pTransform = m_pTransforms + ( m_iTransformCount++ );

	T2GUITransform transform1 = *pTransform;
	T2GUITransform transform2 = a_rTransform;

	TVector2 vec;
	transform1.Transform( vec, a_rVec1 );
	transform1.m_vecTranslation = { vec.x, vec.y };

	transform2.Transform( vec, a_rVec2 );
	transform2.m_vecTranslation = { vec.x, vec.y };

	T2GUITransform::Multiply( m_pTransforms[ m_iTransformCount ], transform1, transform2 );
	m_bIsTransformDirty = TTRUE;
}

void AGUI2RendererDX8::PopTransform()
{
	TASSERT( m_iTransformCount >= 0 );
	m_iTransformCount -= 1;
	m_bIsTransformDirty = TTRUE;
}

void AGUI2RendererDX8::SetTransform( const T2GUITransform& a_rTransform )
{
	m_pTransforms[ m_iTransformCount ] = a_rTransform;
	m_bIsTransformDirty                = TTRUE;
}

void AGUI2RendererDX8::SetColour( TUINT32 a_uiColour )
{
	m_uiColour = a_uiColour;
}

void AGUI2RendererDX8::SetScissor( TFLOAT a_fVal1, TFLOAT a_fVal2, TFLOAT a_fVal3, TFLOAT a_fVal4 )
{
	auto pRender        = TRenderD3DInterface::Interface();
	auto pDisplayParams = pRender->GetCurrentDisplayParams();
	auto pTransform     = m_pTransforms + m_iTransformCount;

	TVector2 transformed1;
	pTransform->Transform( transformed1, { a_fVal1, a_fVal2 } );

	TVector2 transformed2;
	pTransform->Transform( transformed2, { a_fVal3, a_fVal4 } );

	transformed1.x = ( pDisplayParams->uiWidth / 2.0f ) + transformed1.x;
	transformed2.x = ( pDisplayParams->uiWidth / 2.0f ) + transformed2.x;
	transformed1.y = ( pDisplayParams->uiHeight / 2.0f ) - transformed1.y;
	transformed2.y = ( pDisplayParams->uiHeight / 2.0f ) - transformed2.y;

	TINT iLeft   = TMath::Max( TMath::FloorToInt( transformed1.x ), 0 );
	TINT iRight  = TMath::Min( TMath::CeilToInt( transformed2.x ), TINT( pDisplayParams->uiWidth ) );
	TINT iTop    = TMath::Max( TMath::CeilToInt( transformed2.y ), 0 );
	TINT iBottom = TMath::Min( TMath::FloorToInt( transformed1.y ), TINT( pDisplayParams->uiHeight ) );

	D3D11_VIEWPORT viewport = {
		.TopLeftX = TFLOAT( iLeft ),
		.TopLeftY = TFLOAT( iTop ),
		.Width    = TFLOAT( iRight - iLeft ),
		.Height   = TFLOAT( iBottom - iTop ),
		.MinDepth = 0.0f,
		.MaxDepth = 1.0f
	};

	if ( viewport.Width <= 0.0f )
	{
		viewport.Width = 1.0f;
	}

	if ( viewport.Height <= 0.0f )
	{
		viewport.Height = 1.0f;
	}

	pRender->GetDeviceContext()->RSSetViewports( 1, &viewport );

	pRender->SetTransform( TRenderD3DInterface::TRANSFORM_WORLD, TMatrix44::IDENTITY );
	pRender->SetTransform( TRenderD3DInterface::TRANSFORM_VIEW, TMatrix44::IDENTITY );

	TMatrix44 projection;
	SetupProjectionMatrix( projection, iLeft, iRight, iTop, iBottom );
	pRender->SetTransform( TRenderD3DInterface::TRANSFORM_PROJECTION, projection );
}

void AGUI2RendererDX8::ClearScissor()
{
	auto pRender        = TRenderD3DInterface::Interface();
	auto pDisplayParams = pRender->GetCurrentDisplayParams();

	D3D11_VIEWPORT viewport = {
		.TopLeftX = 0.0f,
		.TopLeftY = 0.0f,
		.Width    = TFLOAT( pDisplayParams->uiWidth ),
		.Height   = TFLOAT( pDisplayParams->uiHeight ),
		.MinDepth = 0.0f,
		.MaxDepth = 1.0f
	};

	pRender->GetDeviceContext()->RSSetViewports( 1, &viewport );

	pRender->SetTransform( TRenderD3DInterface::TRANSFORM_WORLD, TMatrix44::IDENTITY );
	pRender->SetTransform( TRenderD3DInterface::TRANSFORM_VIEW, TMatrix44::IDENTITY );

	TMatrix44 projection;
	SetupProjectionMatrix( projection, 0, pDisplayParams->uiWidth, 0, pDisplayParams->uiHeight );
	pRender->SetTransform( TRenderD3DInterface::TRANSFORM_PROJECTION, projection );
}

void AGUI2RendererDX8::RenderRectangle( const Toshi::TVector2& a, const Toshi::TVector2& b, const Toshi::TVector2& uv1, const Toshi::TVector2& uv2 )
{
	if ( m_bIsTransformDirty )
	{
		UpdateTransform();
	}

	sm_Vertices[ 0 ].Position = { a.x, a.y, sm_fZCoordinate };
	sm_Vertices[ 0 ].Colour   = m_uiColour;
	sm_Vertices[ 0 ].UV       = { uv1.x, uv1.y };

	sm_Vertices[ 1 ].Position = { b.x, a.y, sm_fZCoordinate };
	sm_Vertices[ 1 ].Colour   = m_uiColour;
	sm_Vertices[ 1 ].UV       = { uv2.x, uv1.y };

	sm_Vertices[ 2 ].Position = { a.x, b.y, sm_fZCoordinate };
	sm_Vertices[ 2 ].Colour   = m_uiColour;
	sm_Vertices[ 2 ].UV       = { uv1.x, uv2.y };

	sm_Vertices[ 3 ].Position = { b.x, b.y, sm_fZCoordinate };
	sm_Vertices[ 3 ].Colour   = m_uiColour;
	sm_Vertices[ 3 ].UV       = { uv2.x, uv2.y };

	TRenderD3DInterface::Interface()->DrawNonIndexedImmediate( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, sm_Vertices, 4, sizeof( Vertex ) );
}

void AGUI2RendererDX8::RenderTriStrip( Toshi::TVector2* vertices, Toshi::TVector2* UV, uint32_t numverts )
{
	if ( m_bIsTransformDirty )
	{
		UpdateTransform();
	}

#ifdef BARNYARD_LATEST
	// The Wii version also draws strips without UVs (the XUI figure strokes), which can have more vertices
	if ( !UV || numverts > MAX_VERTICES )
	{
		TASSERT( numverts <= MAX_LARGE_VERTICES );
		numverts = TMath::Min( numverts, MAX_LARGE_VERTICES );

		for ( TUINT32 i = 0; i < numverts; i++ )
		{
			sm_aLargeVertices[ i ].Position = { vertices[ i ].x, vertices[ i ].y, sm_fZCoordinate };
			sm_aLargeVertices[ i ].Colour   = m_uiColour;
			sm_aLargeVertices[ i ].UV       = UV ? TVector2( UV[ i ].x, UV[ i ].y ) : TVector2( 0.0f, 0.0f );
		}

		if ( numverts >= 3 )
		{
			TRenderD3DInterface::Interface()->DrawNonIndexedImmediate( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, sm_aLargeVertices, numverts, sizeof( Vertex ) );
		}

		return;
	}
#endif

	for ( TUINT32 i = 0; i < numverts; i++ )
	{
		sm_Vertices[ i ].Position = { vertices[ i ].x, vertices[ i ].y, sm_fZCoordinate };
		sm_Vertices[ i ].Colour   = m_uiColour;
		sm_Vertices[ i ].UV       = { UV[ i ].x, UV[ i ].y };
	}

	TRenderD3DInterface::Interface()->DrawNonIndexedImmediate( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, sm_Vertices, numverts, sizeof( Vertex ) );
}

void AGUI2RendererDX8::RenderLine( const Toshi::TVector2& a, const Toshi::TVector2& b )
{
	RenderLine( a.x, a.y, b.x, b.y );
}

void AGUI2RendererDX8::RenderLine( TFLOAT x1, TFLOAT y1, TFLOAT x2, TFLOAT y2 )
{
	auto pMaterial = m_pMaterial;
	SetMaterial( TNULL );

	if ( m_bIsTransformDirty )
	{
		UpdateTransform();
	}

	sm_Vertices[ 0 ].Position = { x1, y1, 0.0f };
	sm_Vertices[ 0 ].Colour   = m_uiColour;
	sm_Vertices[ 0 ].UV       = { 0.0f, 0.0f };

	sm_Vertices[ 1 ].Position = { x2, y2, 0.0f };
	sm_Vertices[ 1 ].Colour   = m_uiColour;
	sm_Vertices[ 1 ].UV       = { 0.0f, 0.0f };

	TRenderD3DInterface::Interface()->DrawNonIndexedImmediate( D3D11_PRIMITIVE_TOPOLOGY_LINELIST, sm_Vertices, 2, sizeof( Vertex ) );

	SetMaterial( pMaterial );
}

void AGUI2RendererDX8::RenderOutlineRectangle( const Toshi::TVector2& a, const Toshi::TVector2& b )
{
	auto pMaterial = m_pMaterial;
	SetMaterial( TNULL );

	if ( m_bIsTransformDirty )
	{
		UpdateTransform();
	}

	sm_Vertices[ 0 ].Position = { a.x, a.y, sm_fZCoordinate };
	sm_Vertices[ 0 ].Colour   = m_uiColour;
	sm_Vertices[ 0 ].UV       = { 0.0f, 0.0f };

	sm_Vertices[ 1 ].Position = { b.x, a.y, sm_fZCoordinate };
	sm_Vertices[ 1 ].Colour   = m_uiColour;
	sm_Vertices[ 1 ].UV       = { 0.0f, 0.0f };

	sm_Vertices[ 2 ].Position = { b.x, b.y, sm_fZCoordinate };
	sm_Vertices[ 2 ].Colour   = m_uiColour;
	sm_Vertices[ 2 ].UV       = { 0.0f, 0.0f };

	sm_Vertices[ 3 ].Position = { a.x, b.y, sm_fZCoordinate };
	sm_Vertices[ 3 ].Colour   = m_uiColour;
	sm_Vertices[ 3 ].UV       = { 0.0f, 0.0f };

	sm_Vertices[ 4 ].Position = { a.x, a.y, sm_fZCoordinate };
	sm_Vertices[ 4 ].Colour   = m_uiColour;
	sm_Vertices[ 4 ].UV       = { 0.0f, 0.0f };

	TRenderD3DInterface::Interface()->DrawNonIndexedImmediate( D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, sm_Vertices, 5, sizeof( Vertex ) );

	SetMaterial( pMaterial );
}

void AGUI2RendererDX8::RenderFilledRectangle( const Toshi::TVector2& a, const Toshi::TVector2& b )
{
	auto pMaterial = m_pMaterial;
	SetMaterial( TNULL );

	if ( m_bIsTransformDirty )
	{
		UpdateTransform();
	}

	sm_Vertices[ 0 ].Position = { a.x, a.y, sm_fZCoordinate };
	sm_Vertices[ 0 ].Colour   = m_uiColour;
	sm_Vertices[ 0 ].UV       = { 0.0f, 0.0f };

	sm_Vertices[ 1 ].Position = { b.x, a.y, sm_fZCoordinate };
	sm_Vertices[ 1 ].Colour   = m_uiColour;
	sm_Vertices[ 1 ].UV       = { 0.0f, 0.0f };

	sm_Vertices[ 2 ].Position = { a.x, b.y, sm_fZCoordinate };
	sm_Vertices[ 2 ].Colour   = m_uiColour;
	sm_Vertices[ 2 ].UV       = { 0.0f, 0.0f };

	sm_Vertices[ 3 ].Position = { b.x, b.y, sm_fZCoordinate };
	sm_Vertices[ 3 ].Colour   = m_uiColour;
	sm_Vertices[ 3 ].UV       = { 0.0f, 0.0f };

	TRenderD3DInterface::Interface()->DrawNonIndexedImmediate( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, sm_Vertices, 4, sizeof( Vertex ) );

	SetMaterial( pMaterial );
}

void AGUI2RendererDX8::ScaleCoords( TFLOAT& x, TFLOAT& y )
{
	auto pDisplayParams = TRenderInterface::GetSingleton()->GetCurrentDisplayParams();

	TFLOAT fRootWidth, fRootHeight;
	AGUI2::GetContext()->GetRootElement()->GetDimensions( fRootWidth, fRootHeight );

	x /= pDisplayParams->uiWidth / fRootWidth;
	y /= pDisplayParams->uiHeight / fRootHeight;
}

void AGUI2RendererDX8::ResetZCoordinate()
{
	sm_fZCoordinate = 0.01f;
}

void AGUI2RendererDX8::UpdateTransform()
{
	T2GUITransform* pTransform = m_pTransforms + m_iTransformCount;

	TMatrix44 worldMatrix;
	worldMatrix.m_f11 = pTransform->m_aMatrixRows[ 0 ].x;
	worldMatrix.m_f12 = pTransform->m_aMatrixRows[ 0 ].y;
	worldMatrix.m_f13 = 0.0f;
	worldMatrix.m_f14 = 0.0f;

	worldMatrix.m_f21 = pTransform->m_aMatrixRows[ 1 ].x;
	worldMatrix.m_f22 = pTransform->m_aMatrixRows[ 1 ].y;
	worldMatrix.m_f23 = 0.0f;
	worldMatrix.m_f24 = 0.0f;

	worldMatrix.m_f31 = 0.0f;
	worldMatrix.m_f32 = 0.0f;
	worldMatrix.m_f33 = 1.0f;
	worldMatrix.m_f34 = 0.0f;

	worldMatrix.m_f41 = pTransform->m_vecTranslation.x;
	worldMatrix.m_f42 = pTransform->m_vecTranslation.y;
	worldMatrix.m_f43 = 0.0f;
	worldMatrix.m_f44 = 1.0f;

	TRenderD3DInterface::Interface()->SetTransform( TRenderD3DInterface::TRANSFORM_WORLD, worldMatrix );
	m_bIsTransformDirty = TFALSE;
}

void AGUI2RendererDX8::SetupProjectionMatrix( TMatrix44& a_rOutMatrix, TINT a_iLeft, TINT a_iRight, TINT a_iTop, TINT a_iBottom )
{
	auto pDisplayParams = TRenderInterface::GetSingleton()->GetCurrentDisplayParams();

	a_rOutMatrix = {
		2.0f / ( a_iRight - a_iLeft ), 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f / ( a_iBottom - a_iTop ), 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		( ( TINT( pDisplayParams->uiWidth ) - a_iLeft ) - a_iRight ) / (TFLOAT)( a_iRight - a_iLeft ), ( ( TINT( pDisplayParams->uiHeight ) - a_iTop ) - a_iBottom ) / (TFLOAT)( a_iTop - a_iBottom ), 0.0f, 1.0f
	};
}

#ifdef BARNYARD_LATEST

void AGUI2RendererDX8::RenderIndexedTriList( Toshi::TVector2* a_pVertices, Toshi::TVector2* a_pUVs, TINT* a_pIndices, TUINT32 a_uiNumIndices, TUINT32 a_uiNumVertices, TFLOAT a_fScaleX, TFLOAT a_fScaleY )
{
	if ( m_bIsTransformDirty )
		UpdateTransform();

	TASSERT( a_uiNumIndices <= MAX_LARGE_VERTICES );
	TUINT32 uiNumIndices = TMath::Min( a_uiNumIndices, MAX_LARGE_VERTICES );

	for ( TUINT32 i = 0; i < uiNumIndices; i++ )
	{
		TINT iIndex = a_pIndices[ i ];

		sm_aLargeVertices[ i ].Position = { a_pVertices[ iIndex ].x * a_fScaleX, a_pVertices[ iIndex ].y * a_fScaleY, sm_fZCoordinate };
		sm_aLargeVertices[ i ].Colour   = m_uiColour;
		sm_aLargeVertices[ i ].UV       = a_pUVs ? TVector2( a_pUVs[ iIndex ].x, a_pUVs[ iIndex ].y ) : TVector2( 0.0f, 0.0f );
	}

	if ( uiNumIndices >= 3 )
	{
		TRenderD3DInterface::Interface()->DrawNonIndexedImmediate( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, sm_aLargeVertices, uiNumIndices / 3 * 3, sizeof( Vertex ) );
	}
}

#endif // BARNYARD_LATEST
