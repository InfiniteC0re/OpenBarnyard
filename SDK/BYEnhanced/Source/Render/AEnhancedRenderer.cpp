#include "pch.h"
#include "AEnhancedTexture.h"
#include "AEnhancedRenderer.h"
#include "AEnhancedGUIRenderer.h"
#include "ARenderBufferCollection.h"
#include "AEnhancedWorldShader.h"
#include "AEnhancedSkinShader.h"

#include <HookHelpers.h>
#include <Platform/GL/T2Render_GL.h>
#include <Platform/DX8/TRenderInterface_DX8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TBOOL AEnhancedRenderer::Create()
{
	// Setup window parameters
	T2Render::WindowParams oWindowParams;
	oWindowParams.pchTitle    = "Barnyard Enhanced";
	oWindowParams.bIsWindowed = TTRUE;
	oWindowParams.uiWidth     = 800;
	oWindowParams.uiHeight    = 600;

	// Create display
	TBOOL bDisplayCreated = T2Render::CreateSingleton()->Create( oWindowParams );
	TASSERT( TTRUE == bDisplayCreated );

	InstallHooks();

	// Create shaders and initialise other modules here
	AEnhancedTextureManager::CreateSingleton();
	AEnhancedGUIRenderer::CreateSingleton();
	ARenderBufferCollection::CreateSingleton()->Create( 5000 );
	AEnhancedWorldShader::CreateSingleton();
	AEnhancedSkinShader::CreateSingleton();

	return bDisplayCreated;
}

// NOTE: Currently unused
TBOOL AEnhancedRenderer::Update( TFLOAT a_fDeltaTime )
{
	T2Render* pRender = T2Render::GetSingleton();

	// Update renderer and begin drawing scene
	pRender->Update( a_fDeltaTime );
	pRender->BeginScene();

	// ...

	// Rendering is done, so end scene
	pRender->EndScene();

	return TTRUE;
}

MEMBER_HOOK( 0x006c6590, TRenderD3DInterface, TRenderD3DInterface_BeginScene, TBOOL )
{
	if ( CallOriginal() )
	{
		T2Render::GetSingleton()->BeginScene();
		return TTRUE;
	}

	return TFALSE;
}

MEMBER_HOOK( 0x006c6fd0, TRenderD3DInterface, TRenderD3DInterface_EndScene, TBOOL )
{
	if ( CallOriginal() )
	{
		T2Render::GetSingleton()->EndScene();
		return TTRUE;
	}

	return TFALSE;
}

void AEnhancedRenderer::InstallHooks()
{
	InstallHook<TRenderD3DInterface_BeginScene>();
	InstallHook<TRenderD3DInterface_EndScene>();
}
