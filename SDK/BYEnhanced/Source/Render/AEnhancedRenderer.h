#pragma once
#include <Toshi/TSingleton.h>
#include <Platform/GL/T2Render_GL.h>
#include <Platform/GL/T2FrameBuffer_GL.h>
#include <Platform/GL/T2Shader_GL.h>

namespace enhRender
{

inline Toshi::T2FrameBuffer g_FrameBufferDeferred;
inline Toshi::T2Shader      g_ShaderHDR;
inline Toshi::T2Shader      g_ShaderLighting;

} // namespace enhRender

class AEnhancedRenderer
    : public Toshi::TSingleton<AEnhancedRenderer>
{
public:
	AEnhancedRenderer() = default;
	~AEnhancedRenderer() = default;

	TBOOL Create();
	TBOOL Update( TFLOAT a_fDeltaTime );
	
	void ScenePreRender();
	void ScenePostRender();

	void RenderScreenQuad();

private:
	void CreateFrameBuffers();
	void CreateShaders();

	void InstallHooks();

private:
	Toshi::T2Render::WindowParams m_oWindowParams;
};
