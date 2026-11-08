#include "pch.h"
#include "AStaticInstanceMaterialHAL_GL.h"

#include <Render/TOrderTable.h>
#include <Platform/GL/TRenderInterface_GL.h>
#include <Platform/GL/TRenderContext_GL.h>
#include <Platform/GL/TTextureResourceHAL_GL.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AStaticInstanceMaterialHAL );

// NOTE: inlined in AStaticInstanceShaderHAL::CreateMaterial and the create functions of the class
AStaticInstanceMaterialHAL::AStaticInstanceMaterialHAL()
{
}

AStaticInstanceMaterialHAL::~AStaticInstanceMaterialHAL()
{
	if ( GetRegMaterial() )
		TOrderTable::DeregisterMaterial( GetRegMaterial() );
}

void AStaticInstanceMaterialHAL::PreRender()
{
	TRenderGLInterface* pRenderInterface = TRenderGLInterface::Interface();
	TTextureResourceHAL* pTextureHAL      = TSTATICCAST( TTextureResourceHAL, m_aTextures[ 0 ] );

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
			pRenderInterface->SetSrcBlend( GL_ONE );
			pRenderInterface->SetDestBlend( GL_ZERO );
			break;
		case BLENDMODE_MODULATE:
			pRenderInterface->SetSrcBlend( GL_ZERO );
			pRenderInterface->SetDestBlend( GL_SRC_COLOR );
			break;
		case BLENDMODE_ADDITIVE:
			pRenderInterface->SetSrcBlend( GL_SRC_ALPHA );
			pRenderInterface->SetDestBlend( GL_ONE );
			break;
		default:
			pRenderInterface->SetSrcBlend( GL_SRC_ALPHA );
			pRenderInterface->SetDestBlend( GL_ONE_MINUS_SRC_ALPHA );
			break;
	}

	pRenderInterface->SetZWriteEnable( TTRUE );

	TRenderContextGL* pCurrentContext = TRenderContextGL::Upcast( pRenderInterface->GetCurrentContext() );

	if ( pCurrentContext->IsFogEnabled() )
		pCurrentContext->EnableFogHAL();
}

void AStaticInstanceMaterialHAL::PostRender()
{
	TRenderGLInterface* pRenderInterface = TRenderGLInterface::Interface();

	pRenderInterface->SetTexture( 0, 0 );

	if ( m_Flags & FLAGS_NO_CULL )
		pRenderInterface->SetCullMode( TRenderGLInterface::CULL_FRONT );

	pRenderInterface->SetTextureAddress( 0, ADDRESSINGMODE_WRAP, TEXCOORD_UV );
	pRenderInterface->SetTexture( 0, 0 );
	pRenderInterface->SetZWriteEnable( TTRUE );
	pRenderInterface->SetFogEnable( TFALSE );
}
