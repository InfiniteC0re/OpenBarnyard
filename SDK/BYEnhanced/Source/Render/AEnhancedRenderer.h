#pragma once
#include <Toshi/TSingleton.h>
#include <Toshi/T2Vector.h>
#include <ToshiTools/T2DynamicVector.h>
#include <Platform/GL/T2Render_GL.h>
#include <Platform/GL/T2FrameBuffer_GL.h>
#include <Platform/GL/T2RenderBuffer_GL.h>
#include <Platform/GL/T2Shader_GL.h>

#include <glm/ext.hpp>

namespace enhRender
{

inline Toshi::T2FrameBuffer g_FrameBufferDeferred;
inline Toshi::T2FrameBuffer g_ShadowMap1;
inline Toshi::T2Shader      g_ShaderLighting;
inline Toshi::TVector3      g_DirectionalLightDir( 0.653f, 0.757f, 0.0f );
inline Toshi::TVector3      g_FogColor( 1.0f, 1.0f, 1.0f );
inline Toshi::TMatrix44     g_Projection;
inline Toshi::TMatrix44     g_LightViewMatrix;

inline TFLOAT g_ShadowBiasMin = 0.0005f;
inline TFLOAT g_ShadowBiasMax = 0.0047f;
inline TFLOAT g_ShadowStrength = 0.2f;

inline TFLOAT g_Exposure = 6.4f;
inline TFLOAT g_Gamma = 2.2f;

} // namespace enhRender

class AEnhancedRenderer
    : public Toshi::TSingleton<AEnhancedRenderer>
{
public:
	AEnhancedRenderer();
	~AEnhancedRenderer() = default;

	TBOOL Create();
	TBOOL Update( TFLOAT a_fDeltaTime );

	void ResetMultiDraw();
	void AddMultiDrawCommand( TUINT a_uiCount, TUINT a_uiInstanceCount, TUINT a_uiFirstIndex, TINT a_iBaseVertex, TUINT a_uiBaseInstance );
	TINT AddMultiDrawModelViewMatrix( const Toshi::TMatrix44& a_rcModelView );
	void FlushMultiDrawCommands();
	TINT GetNumMultiDrawCommands() const { return m_vecMultiDrawCommands.Size(); }
	DrawElementsIndirectCommand* GetPrevMultiDrawCommand();

	TINT GetMultiDrawCursor() const { return m_iMultiDrawCursor; }
	void SeekMultiDrawCursor( TINT a_iOffset ) { m_iMultiDrawCursor += a_iOffset; }
	void* GetMultiDrawCommandOffset() const { return (void*)( m_iMultiDrawCursor * sizeof( DrawElementsIndirectCommand ) ); }

	const Toshi::T2IndirectBuffer&      GetIndirectBuffer() const { return m_oIndirectBuffer; }
	const Toshi::T2ShaderStorageBuffer& GetShaderStorageBuffer() const { return m_oMultiDrawShaderStorage; }

	void ScenePreRender();
	void ScenePostRender();

	void RenderScreenQuad();

private:
	void CreateFrameBuffers();
	void CreateShaders();

	void InstallHooks();

private:
	Toshi::T2Render::WindowParams m_oWindowParams;

	Toshi::T2DynamicVector<DrawElementsIndirectCommand> m_vecMultiDrawCommands;

	struct MultiDrawData
	{
		TINT             indices[ 8000 ];
		Toshi::TMatrix44 matrices[ 8000 ];
	} m_oMultiDrawData;

	TINT m_iMultiDrawNumIndices = 0;
	TINT m_iMultiDrawNumMatrices = 0;

	TBOOL                                               m_bMultiDrawDirty = TTRUE;
	Toshi::T2IndirectBuffer                             m_oIndirectBuffer;
	Toshi::T2ShaderStorageBuffer                        m_oMultiDrawShaderStorage;
	TINT                                                m_iMultiDrawCursor;
};
