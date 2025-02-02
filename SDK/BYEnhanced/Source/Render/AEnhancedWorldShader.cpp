#include "pch.h"
#include "HookHelpers.h"
#include "AEnhancedWorldShader.h"
#include "AEnhancedTexture.h"
#include "ARenderBufferCollection.h"

#include <Platform/GL/T2Render_GL.h>
#include <Platform/DX8/TIndexPoolResource_DX8.h>
#include <Platform/DX8/TVertexPoolResource_DX8.h>
#include <Platform/DX8/TTextureResourceHAL_DX8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AEnhancedWorldShader::AEnhancedWorldShader()
{
	// Compile shaders
	m_hVertexShader   = T2Render::CompileShaderFromFile( GL_VERTEX_SHADER, "Shaders/WorldShader.vs" );
	m_hFragmentShader = T2Render::CompileShaderFromFile( GL_FRAGMENT_SHADER, "Shaders/WorldShader.fs" );

	// Create shader program
	m_oShaderProgram = T2Render::CreateShaderProgram( m_hVertexShader, m_hFragmentShader );
	
	InstallHooks();
}

AEnhancedWorldShader::~AEnhancedWorldShader()
{
}

void AEnhancedWorldShader::PreRender()
{
	auto pShadowColor   = (Toshi::TVector4*)( *(TUINT*)( 0x0079a854 ) + 0xF0 );
	auto pAmbientColor  = (Toshi::TVector4*)( *(TUINT*)( 0x0079a854 ) + 0x100 );
	auto pRenderContext = TSTATICCAST(
	    Toshi::TRenderContext,
	    THookedRenderD3DInterface::GetSingleton()->GetCurrentContext()
	);

	m_oShaderProgram.Use();
	m_oShaderProgram.SetUniform( "u_Color", Toshi::TVector4( 1.0f, 1.0f, 1.0f, 1.0f ) );
	m_oShaderProgram.SetUniform( "u_ShadowColor", *pShadowColor );
	m_oShaderProgram.SetUniform( "u_AmbientColor", *pAmbientColor );
	m_oShaderProgram.SetUniform( "u_Projection", *(Toshi::TMatrix44*)( TUINT( pRenderContext ) + 0x3C0 ) );

	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );

	glEnable( GL_DEPTH_TEST );
	glPolygonMode( GL_BACK, GL_FILL );
}

void AEnhancedWorldShader::Render( Toshi::TRenderPacket* a_pRenderPacket )
{
	// TODO: Use single VAO for better performance
	// Also, consider combining VBOs and IBOs into a single big buffer to reduce changes in context

	AWorldMesh* pMesh       = (AWorldMesh*)a_pRenderPacket->GetMesh();
	auto        pIndexPool  = *TREINTERPRETCAST( Toshi::TIndexPoolResource**, ( *TREINTERPRETCAST( TUINT*, TUINT( pMesh ) + 0x1C ) ) + 0x8 );

	m_oShaderProgram.SetUniform( "u_View", a_pRenderPacket->GetModelViewMatrix() );

	ARenderBuffer renderBuffer = ARenderBufferCollection::GetSingleton()->GetRenderBuffer( pMesh->m_pSubMeshes[ 0 ].iRenderBufferId );
	renderBuffer->Bind();

	glDrawElements( GL_TRIANGLE_STRIP, pIndexPool->GetNumIndices(), GL_UNSIGNED_SHORT, TNULL );
}

class AWorldMaterial
	: public Toshi::TMaterial
{
public:
	static constexpr TUINT MAX_TEXTURES = 4;

	using BLENDMODE = TINT;

public:
	Toshi::TTexture* m_aTextures[ MAX_TEXTURES ];
	BLENDMODE        m_eBlendMode;
	TFLOAT           m_fUVAnimX;
	TFLOAT           m_fUVAnimY;
	TFLOAT           m_fUVAnimSpeedX;
	TFLOAT           m_fUVAnimSpeedY;
};

MEMBER_HOOK( 0x005f6f70, AWorldMaterial, AWorldMaterial_PreRender, void )
{
	CallOriginal();

	TTextureResourceHAL* pTexture = TSTATICCAST( TTextureResourceHAL, m_aTextures[ 0 ] );
	T2GLTexture*         pGLTexture = AEnhancedTextureManager::GetAssociation( pTexture->GetD3DTexture() );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, pGLTexture->GetHandle() );
}

class AWorldShaderHAL
{};

MEMBER_HOOK( 0x005f6510, AWorldShaderHAL, AWorldShaderHAL_StartFlush, void )
{
	CallOriginal();
	AEnhancedWorldShader::GetSingleton()->PreRender();
}

MEMBER_HOOK( 0x005f6cb0, AWorldShaderHAL, AWorldShaderHAL_Render, void, Toshi::TRenderPacket* a_pRenderPacket )
{
	if ( *(TBOOL*)( TUINT( this ) + 0xEC ) == TFALSE )
	{
		return;
	}

	AEnhancedWorldShader::GetSingleton()->Render( a_pRenderPacket );
	CallOriginal( a_pRenderPacket );
}

void AEnhancedWorldShader::InstallHooks()
{
	InstallHook<AWorldMaterial_PreRender>();
	InstallHook<AWorldShaderHAL_StartFlush>();
	InstallHook<AWorldShaderHAL_Render>();
}
