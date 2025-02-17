#pragma once
#include <Toshi/TSingleton.h>
#include <Platform/GL/T2Render_GL.h>
#include <Platform/GL/T2FrameBuffer_GL.h>
#include <Platform/GL/T2Shader_GL.h>

#include <glm/ext.hpp>

namespace enhRender
{

inline Toshi::T2FrameBuffer g_FrameBufferDeferred;
inline Toshi::T2FrameBuffer g_ShadowMap1;
inline Toshi::T2Shader      g_ShaderHDR;
inline Toshi::T2Shader      g_ShaderLighting;
inline Toshi::TVector3      g_DirectionalLightDir( 0.653f, 0.757f, 0.0f );
inline Toshi::TVector3      g_FogColor( 1.0f, 1.0f, 1.0f );
inline Toshi::TMatrix44     g_Projection;
inline Toshi::TMatrix44     g_LightViewMatrix;

inline TFLOAT g_ShadowBiasMin = 0.0005f;
inline TFLOAT g_ShadowBiasMax = 0.0047f;
inline TFLOAT g_ShadowStrength = 0.2f;

} // namespace enhRender

typedef struct
{
	TUINT count;
	TUINT instanceCount;
	TUINT firstIndex;
	TINT  baseVertex;
	TUINT baseInstance;
} DrawElementsIndirectCommand;

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
