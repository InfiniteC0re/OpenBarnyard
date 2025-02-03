#include "pch.h"
#include "AEnhancedRenderer.h"
#include "AEnhancedGUIRenderer.h"
#include "AEnhancedTexture.h"

#include <HookHelpers.h>
#include <BYardSDK/AGUI2Renderer.h>
#include <BYardSDK/AGUI2Material.h>
#include <Platform/DX8/TTextureResourceHAL_DX8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AEnhancedGUIRenderer::AEnhancedGUIRenderer()
{
	// Compile shaders
	m_hVertexShader = T2Render::CompileShaderFromFile( GL_VERTEX_SHADER, "Shaders/AGUI2.vs" );
	m_hFragmentShader = T2Render::CompileShaderFromFile( GL_FRAGMENT_SHADER, "Shaders/AGUI2.fs" );
	
	// Create shader program
	m_oShaderProgram = T2Render::CreateShaderProgram( m_hVertexShader, m_hFragmentShader );
	
	CreateVertexArray();
	InstallHooks();
}

AEnhancedGUIRenderer::~AEnhancedGUIRenderer()
{

}

void AEnhancedGUIRenderer::BeginScene()
{
	TUINT uiWidth  = T2Render::GetSingleton()->GetWindowParams().uiWidth;
	TUINT uiHeight = T2Render::GetSingleton()->GetWindowParams().uiHeight;

	TMatrix44 matProjection = {
		2.0f / uiWidth, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f / uiHeight, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	T2Render::SetShaderProgram( m_oShaderProgram );
	m_oShaderProgram.SetUniform( "u_Projection", matProjection );

	m_oVertexArray.Bind();
	m_oVertexArray.GetVertexBuffer().Bind();

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glCullFace( GL_FRONT );
	glDisable( GL_DEPTH_TEST );
}

void AEnhancedGUIRenderer::EndScene()
{
	m_oVertexArray.Unbind();
}

void AEnhancedGUIRenderer::RenderRectangle( const Toshi::TVector2& a, const Toshi::TVector2& b, const Toshi::TVector2& uv1, const Toshi::TVector2& uv2 )
{
	TFLOAT fZ = *(TFLOAT*)0x00779938 * -1.0f;

	ms_aVertices[ 0 ].Position.x = a.x;
	ms_aVertices[ 0 ].Position.y = a.y;
	ms_aVertices[ 0 ].Position.z = fZ;
	ms_aVertices[ 0 ].Colour     = m_uiColour;
	ms_aVertices[ 0 ].UV.x       = uv1.x;
	ms_aVertices[ 0 ].UV.y       = uv1.y;
	ms_aVertices[ 1 ].Position.y = a.y;
	ms_aVertices[ 1 ].Position.x = b.x;
	ms_aVertices[ 1 ].Position.z = fZ;
	ms_aVertices[ 1 ].Colour     = m_uiColour;
	ms_aVertices[ 1 ].UV.y       = uv1.y;
	ms_aVertices[ 1 ].UV.x       = uv2.x;
	ms_aVertices[ 2 ].Position.y = b.y;
	ms_aVertices[ 2 ].Position.x = a.x;
	ms_aVertices[ 2 ].Position.z = fZ;
	ms_aVertices[ 2 ].Colour     = m_uiColour;
	ms_aVertices[ 2 ].UV.y       = uv2.y;
	ms_aVertices[ 2 ].UV.x       = uv1.x;
	ms_aVertices[ 3 ].Position.y = b.y;
	ms_aVertices[ 3 ].Position.x = b.x;
	ms_aVertices[ 3 ].Position.z = fZ;
	ms_aVertices[ 3 ].Colour     = m_uiColour;
	ms_aVertices[ 3 ].UV.y       = uv2.y;
	ms_aVertices[ 3 ].UV.x       = uv2.x;

	m_oVertexArray.GetVertexBuffer().SetData( &ms_aVertices, sizeof( AEnhancedGUIRenderer::Vertex ) * 4, GL_DYNAMIC_DRAW );

	AGUI2Transform* pTransform = m_pTransforms + m_iTransformCount;

	TMatrix44 view;
	view.m_f11 = pTransform->m_aMatrixRows[ 0 ].x;
	view.m_f12 = pTransform->m_aMatrixRows[ 0 ].y;
	view.m_f13 = 0.0f;
	view.m_f14 = 0.0f;
	view.m_f21 = pTransform->m_aMatrixRows[ 1 ].x;
	view.m_f22 = pTransform->m_aMatrixRows[ 1 ].y;
	view.m_f23 = 0.0f;
	view.m_f24 = 0.0f;
	view.m_f31 = 0.0f;
	view.m_f32 = 0.0f;
	view.m_f33 = 1.0f;
	view.m_f34 = 0.0f;
	view.m_f41 = pTransform->m_vecTranslation.x;
	view.m_f42 = pTransform->m_vecTranslation.y;
	view.m_f43 = 0.0f;
	view.m_f44 = 1.0f;
	m_oShaderProgram.SetUniform( "u_View", view );

	glDrawElements( GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, TNULL );
}

void AEnhancedGUIRenderer::SetColour( TUINT32 a_uiColour )
{
	m_uiColour = a_uiColour;
}

void AEnhancedGUIRenderer::SetTransforms( AGUI2Transform* a_pTransforms, TINT a_iNumTransforms )
{
	m_pTransforms     = a_pTransforms;
	m_iTransformCount = a_iNumTransforms;
}

void AEnhancedGUIRenderer::CreateVertexArray()
{
	TUINT16 aIndices[] = { 0, 1, 2, 3 };

	m_oVertexArray = T2Render::CreateVertexArray(
	    T2Render::CreateVertexBuffer( TNULL, 0, GL_DYNAMIC_DRAW ),
	    T2Render::CreateIndexBuffer( aIndices, TARRAYSIZE( aIndices ), GL_STATIC_DRAW )
	);

	m_oVertexArray.SetAttribPointer( 0, 3, GL_FLOAT, sizeof( AEnhancedGUIRenderer::Vertex ), 0 );
	m_oVertexArray.SetAttribPointer( 1, 4, GL_UNSIGNED_BYTE, sizeof( AEnhancedGUIRenderer::Vertex ), (void*)offsetof( AEnhancedGUIRenderer::Vertex, Colour ), GL_TRUE );
	m_oVertexArray.SetAttribPointer( 2, 2, GL_FLOAT, sizeof( AEnhancedGUIRenderer::Vertex ), (void*)offsetof( AEnhancedGUIRenderer::Vertex, UV ) );
}

// Hooks

class AGUI2RendererDX8
    : public AGUI2Renderer
{
public:
	AGUI2Transform* m_pTransforms;
	TINT            m_iTransformCount;
	TUINT32         m_uiColour;
	TBOOL           m_bIsTransformDirty;
	AGUI2Material*  m_pMaterial;
};

MEMBER_HOOK( 0x0064eb90, AGUI2RendererDX8, AGUI2Renderer_SetMaterial, void, AGUI2Material* a_pMaterial )
{
	if ( m_pMaterial != a_pMaterial )
	{
		CallOriginal( a_pMaterial );

		if ( a_pMaterial )
		{
			auto pTextureResource = TSTATICCAST( TTextureResourceHAL, a_pMaterial->m_pTextureResource );

			if ( pTextureResource )
			{
				T2Render::SetTexture2D( 0, AEnhancedTextureManager::GetAssociation( pTextureResource->GetD3DTexture() ) );
			}
		}
		else
		{
			T2Render::ResetTexture2D( 0 );
		}
	}
}

MEMBER_HOOK( 0x0064fa70, AGUI2RendererDX8, AGUI2Renderer_RenderRectangle, void, const Toshi::TVector2& a, const Toshi::TVector2& b, const Toshi::TVector2& uv1, const Toshi::TVector2& uv2 )
{
	AEnhancedGUIRenderer::GetSingleton()->SetTransforms( m_pTransforms, m_iTransformCount );
	AEnhancedGUIRenderer::GetSingleton()->SetColour( m_uiColour );
	AEnhancedGUIRenderer::GetSingleton()->RenderRectangle( a, b, uv1, uv2 );

	CallOriginal( a, b, uv1, uv2 );
}

MEMBER_HOOK( 0x0064f490, AGUI2RendererDX8, AGUI2Renderer_BeginScene, void )
{
	CallOriginal();
	AEnhancedGUIRenderer::GetSingleton()->BeginScene();
}

MEMBER_HOOK( 0x0064e7c0, AGUI2RendererDX8, AGUI2Renderer_EndScene, void )
{
	CallOriginal();
	AEnhancedGUIRenderer::GetSingleton()->EndScene();
}

void AEnhancedGUIRenderer::InstallHooks()
{
	InstallHook<AGUI2Renderer_SetMaterial>();
	InstallHook<AGUI2Renderer_RenderRectangle>();
	InstallHook<AGUI2Renderer_BeginScene>();
	InstallHook<AGUI2Renderer_EndScene>();
}
