#pragma once

#include <Toshi/TSingleton.h>
#include <Render/TIndexPoolResourceInterface.h>
#include <Render/TVertexPoolResourceInterface.h>
#include <Platform/GL/T2RenderBuffer_GL.h>
#include <Platform/GL/T2Shader_GL.h>

class AWorldMesh
	: public Toshi::TMesh
{
public:
	struct SubMesh
	{
		TINT                                iRenderBufferId;
		TUINT16                             uiUnk2;
		TUINT16                             uiNumVertices;
		Toshi::TIndexPoolResourceInterface* pIndexPool;
	};

	static constexpr TUINT NUM_SUBMESHES = 1;
	using LockBuffer                     = Toshi::TVertexPoolResourceInterface::LockBuffer;

public:
	SubMesh* GetSubMesh( TUINT a_uiIndex )
	{
		TASSERT( a_uiIndex < NUM_SUBMESHES );
		return &m_pSubMeshes[ a_uiIndex ];
	}

public:
	TUINT32                              m_uiFlags;
	TUINT16                              m_uiMaxVertices;
	Toshi::TVertexPoolResourceInterface* m_pVertexPool;
	SubMesh*                             m_pSubMeshes;
};

class AEnhancedWorldShader
    : public Toshi::TSingleton<AEnhancedWorldShader>
{
public:
	AEnhancedWorldShader();
	~AEnhancedWorldShader();

	void StartFlush();
	void Render( Toshi::TRenderPacket* a_pRenderPacket );

private:
	void InstallHooks();

private:
	Toshi::T2CompiledShader m_hVertexShader;
	Toshi::T2CompiledShader m_hFragmentShader;
	Toshi::T2Shader         m_oShaderProgram;
};
