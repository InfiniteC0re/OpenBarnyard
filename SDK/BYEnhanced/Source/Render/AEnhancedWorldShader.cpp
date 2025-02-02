#include "pch.h"
#include "HookHelpers.h"
#include "AEnhancedWorldShader.h"
#include "ARenderBufferCollection.h"

#include <Platform/GL/T2Render_GL.h>
#include <Platform/DX8/TIndexPoolResource_DX8.h>
#include <Platform/DX8/TVertexPoolResource_DX8.h>

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

void AEnhancedWorldShader::StartFlush()
{
	
}

void AEnhancedWorldShader::Render( Toshi::TRenderPacket* a_pRenderPacket )
{
	AWorldMesh* pMesh       = (AWorldMesh*)a_pRenderPacket->GetMesh();
	auto        pVertexPool = *TREINTERPRETCAST( Toshi::TVertexPoolResource**, TUINT( pMesh ) + 0x18 );
	auto        pIndexPool  = *TREINTERPRETCAST( Toshi::TIndexPoolResource**, ( *TREINTERPRETCAST( TUINT*, TUINT( pMesh ) + 0x1C ) ) + 0x8 );

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
	m_oShaderProgram.SetUniform( "u_View", a_pRenderPacket->GetModelViewMatrix() );

	ARenderBuffer renderBuffer = ARenderBufferCollection::GetSingleton()->GetRenderBuffer( pMesh->m_pSubMeshes[ 0 ].iRenderBufferId );
	renderBuffer->Bind();

	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );

	glEnable( GL_DEPTH_TEST );
	glPolygonMode( GL_BACK, GL_FILL );

	glDrawElements( GL_TRIANGLE_STRIP, pIndexPool->GetNumIndices(), GL_UNSIGNED_SHORT, TNULL );

	renderBuffer->Unbind();
}

//MEMBER_HOOK( 0x005f6f70, AWorldMaterialWrapperGL, AWorldMaterial_PreRender, void )
//{
//	CallOriginal();
//	AWorldMaterialWrapperGL::PreRenderGL();
//}

class AWorldShaderHAL
{};

MEMBER_HOOK( 0x005f6510, AWorldShaderHAL, AWorldShaderHAL_StartFlush, void )
{
	CallOriginal();
	//AWorldShaderGL::GetSingleton()->StartFlush();
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
	//InstallHook<AWorldMaterial_PreRender>();
	InstallHook<AWorldShaderHAL_StartFlush>();
	InstallHook<AWorldShaderHAL_Render>();
}
