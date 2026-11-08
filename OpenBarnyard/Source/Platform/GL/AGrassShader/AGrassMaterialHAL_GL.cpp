#include "pch.h"
#include "AGrassMaterialHAL_GL.h"
#include "AGrassShaderHAL_GL.h"

#include <Platform/GL/TRenderInterface_GL.h>
#include <Platform/GL/TRenderContext_GL.h>
#include <Platform/GL/TTextureResourceHAL_GL.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGrassMaterialHAL );

AGrassMaterialHAL::AGrassMaterialHAL()
{
}

AGrassMaterialHAL::~AGrassMaterialHAL()
{
}

void AGrassMaterialHAL::PreRender()
{
	TRenderGLInterface* pRenderInterface = TRenderGLInterface::Interface();
	TRenderContextGL*   pCurrentContext  = TRenderContextGL::Upcast( pRenderInterface->GetCurrentContext() );
	TTextureResourceHAL* pTextureHAL      = TSTATICCAST( TTextureResourceHAL, m_aTextures[ 0 ] );

	g_pGrassShaderTerrainTexture = TSTATICCAST( TTextureResourceHAL, m_aTextures[ 0 ] );

	if ( pTextureHAL )
	{
		pTextureHAL->Validate();

		if ( pTextureHAL->GetGLTexture() )
		{
			pRenderInterface->SetTexture( 0, pTextureHAL->GetGLTexture() );
			pRenderInterface->SetTextureAddress( 0, pTextureHAL->GetAddressing(), TEXCOORD_UV );
		}
	}

	if ( m_Flags & FLAGS_NO_CULL )
		pRenderInterface->SetCullMode( TRenderGLInterface::CULL_NONE );

	switch ( m_eBlendMode )
	{
		case BLENDMODE_DEFAULT:
			pRenderInterface->SetSrcBlend( GL_SRC_ALPHA );
			pRenderInterface->SetDestBlend( GL_ONE_MINUS_SRC_ALPHA );
			pRenderInterface->SetZWriteEnable( TTRUE );
			break;
		case BLENDMODE_ALPHA:
			pRenderInterface->SetSrcBlend( GL_SRC_ALPHA );
			pRenderInterface->SetDestBlend( GL_ONE_MINUS_SRC_ALPHA );
			pRenderInterface->SetZWriteEnable( TFALSE );
			break;
		case BLENDMODE_MODULATE:
			pRenderInterface->SetSrcBlend( GL_ZERO );
			pRenderInterface->SetDestBlend( GL_SRC_COLOR );
			pRenderInterface->SetZWriteEnable( TFALSE );
			break;
		case BLENDMODE_ADDITIVE:
			pRenderInterface->SetSrcBlend( GL_SRC_ALPHA );
			pRenderInterface->SetDestBlend( GL_ONE );
			pRenderInterface->SetZWriteEnable( TFALSE );
			break;
	}

	pCurrentContext->EnableFogHAL();
}

void AGrassMaterialHAL::PostRender()
{
	TRenderGLInterface* pRenderInterface = TRenderGLInterface::Interface();

	if ( m_Flags & FLAGS_NO_CULL )
		pRenderInterface->SetCullMode( TRenderGLInterface::CULL_FRONT );

	pRenderInterface->SetTextureAddress( 0, ADDRESSINGMODE_WRAP, TEXCOORD_UV );
	pRenderInterface->SetTexture( 0, 0 );
	pRenderInterface->SetZWriteEnable( TTRUE );
}
