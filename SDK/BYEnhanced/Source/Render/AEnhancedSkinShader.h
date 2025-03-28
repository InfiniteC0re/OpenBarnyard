#pragma once
#include "ARenderBufferCollection.h"

#include <Toshi/TSingleton.h>
#include <Render/TMesh.h>
#include <Render/TIndexPoolResourceInterface.h>
#include <Render/TVertexPoolResourceInterface.h>

#include <Platform/GL/T2Shader_GL.h>
#include <Platform/GL/T2GLTexture_GL.h>

class ASkinMesh
	: public Toshi::TMesh
{
public:
	using LockBuffer = Toshi::TVertexPoolResourceInterface::LockBuffer;

	struct SubMesh
	{
		TUINT16                             uiUnknown;
		TUINT16                             uiNumVertices;
		Toshi::TIndexPoolResourceInterface* pIndexPool;
		TUINT32                             Zero;
		TUINT32                             Unk2;
		TUINT32                             Unk3;
		TUINT32                             Unk4;
		TUINT32                             Unk5;
		TUINT32                             Unk6;
		TUINT32                             uiNumBones;
		TINT                                aBones[ 28 ];
	};

	struct SkinVertex
	{
		Toshi::TVector3 Position;
		Toshi::TVector3 Normal;
		TUINT8          Weights[ 4 ];
		TUINT8          Bones[ 4 ];
		Toshi::TVector2 UV;
	};

public:
	TUINT32                              m_uiFlags;
	TUINT16                              m_uiMaxVertices;
	TUINT16                              m_uiNumSubMeshes;
	Toshi::TVertexPoolResourceInterface* m_pVertexPool;
	SubMesh*                             m_pSubMeshes;
	void*                                m_Unk;
};

class AEnhancedSkinShader
    : public Toshi::TSingleton<AEnhancedSkinShader>
{
public:
	inline static constexpr TSIZE MULTIDRAW_MAX = 64;

public:
	AEnhancedSkinShader();
	~AEnhancedSkinShader();
	
	void PreRender();
	void Render( Toshi::TRenderPacket* a_pRenderPacket );

	// Saves information about the mesh into an indirect buffer
	void CreateMultiDrawCommand( Toshi::TRenderPacket* a_pRenderPacket );

	void QueueMultiDraw( const ARenderBuffer& a_rcRenderBuffer );
	void FlushMultiDraw();

private:
	void InstallHooks();

private:
	Toshi::T2CompiledShader m_hVertexShader;
	Toshi::T2CompiledShader m_hFragmentShader;
	Toshi::T2Shader         m_oShaderProgram;

	Toshi::TMatrix44 m_WorldViewMatrix;
	Toshi::TMatrix44 m_ViewWorldMatrix;

	ARenderBuffer m_oMultiDrawLastBuffer;
	TINT          m_iMultiDrawSize = 0;
};
