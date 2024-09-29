#pragma once
#include "TRender_SDL.h"
#include "TTexture_SDL.h"

#include <Render/TTexture.h>
#include <Platform/DX8/TTextureResourceHAL_DX8.h>

class AWorldMaterialWrapperGL :
    public Toshi::TMaterial
{
public:
	void PreRenderGL()
	{
		if ( m_iNumTex > 0 )
		{
			auto pD3DTexture = m_aTextures[ 0 ]->GetD3DTexture();

			if ( !m_pTextures[ 0 ] || pD3DTexture != (IDirect3DTexture8*)m_pTextures[ 1 ] )
			{
				auto pInvalidNative = THookedRenderD3DInterface::GetSingleton()->GetInvalidTexture();

				if ( m_aTextures[ 0 ] != pInvalidNative )
				{
					auto pTexManagerSDL = Toshi::TTextureManagerSDL::GetSingleton();
					m_pTextures[ 0 ]    = TREINTERPRETCAST( Toshi::TTexture*, pTexManagerSDL->GetGLAssociation( pD3DTexture ) );
					m_pTextures[ 1 ]    = TREINTERPRETCAST( Toshi::TTexture*, pD3DTexture );
				}
			}

			glActiveTexture( GL_TEXTURE0 );

			if ( m_pTextures[ 0 ] )
			{
				auto pGLTexture = TREINTERPRETCAST( Toshi::TTextureSDL*, m_pTextures[ 0 ] );
				glBindTexture( GL_TEXTURE_2D, (GLuint)pGLTexture->GetHandle() );
			}
			else
			{
				glBindTexture( GL_TEXTURE_2D, (GLuint)Toshi::TTextureManagerSDL::GetSingleton()->GetInvalidTexture()->GetHandle() );
			}
		}
		else
		{
			glBindTexture( GL_TEXTURE_2D, (GLuint)Toshi::TTextureManagerSDL::GetSingleton()->GetInvalidTexture()->GetHandle() );
		}

		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

		//if (m_iBlendMode == 0)
		//{
		//	// 2 = GL_ONE
		//	// 5 = GL_SRC_ALPHA
		//	// 6 = GL_ONE_MINUS_SRC_ALPHA
		//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//	glDepthMask(GL_TRUE);
		//}
		//else
		//{
		//	if (m_iBlendMode == 1)
		//	{
		//		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//		glDepthMask(GL_FALSE);
		//	}
		//	else if (m_iBlendMode == 3)
		//	{
		//		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		//		glDepthMask(GL_FALSE);
		//	}
		//	else
		//	{
		//		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//		glDepthMask(GL_TRUE);
		//	}
		//}
	}

private:
	Toshi::TTextureResourceHAL* m_aTextures[ 4 ];
	TINT                        m_iBlendMode; // ?
};

class AWorldShaderGL :
    public Toshi::TSingleton<AWorldShaderGL>
{
public:
	struct WorldVertex
	{
		Toshi::TVector3 Position;
		Toshi::TVector3 Normal;
		Toshi::TVector3 Color;
		Toshi::TVector2 UV;
	};

public:
	AWorldShaderGL()
	{
		m_VAO = Toshi::TRenderSDL::CreateVertexArray(
		    Toshi::TRenderSDL::CreateVertexBuffer( TNULL, 0, GL_DYNAMIC_DRAW ),
		    Toshi::TRenderSDL::CreateIndexBuffer( TNULL, 0, GL_DYNAMIC_DRAW )
		);

		m_VertexShader = Toshi::TRenderSDL::CompileShader(
		    GL_VERTEX_SHADER,
		    "#version 400\n\
			\
			layout(location = 0) in vec3 a_Position;\n\
			layout(location = 1) in vec3 a_Normal;\n\
			layout(location = 2) in vec3 a_Color;\n\
			layout(location = 3) in vec2 a_UV;\n\
			\
			out vec2 textureCoord;\n\
			out vec3 vertexColor;\n\
			\
			uniform mat4 u_Projection;\n\
			uniform mat4 u_View;\n\
			\
			void main()\n\
			{\n\
				textureCoord = a_UV;\n\
				vertexColor = a_Color;\n\
				gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);\n\
			}"
		);

		m_PixelShader = Toshi::TRenderSDL::CompileShader(
		    GL_FRAGMENT_SHADER,
		    "#version 400\n\
			in vec2 textureCoord;\n\
			in vec3 vertexColor;\n\
			\
			out vec4 color;\n\
			\
			uniform vec4 u_Color;\n\
			uniform vec4 u_ShadowColor;\n\
			uniform vec4 u_AmbientColor;\n\
			uniform sampler2D tex0;\n\
			\
			void main()\n\
			{\n\
				vec4 texColor = texture(tex0, textureCoord);\n\
				if (texColor.a < 0.2) discard;\n\
				vec3 shadow = vertexColor;\n\
				color = texColor * mix(u_AmbientColor, u_ShadowColor, vec4(1 - vertexColor, 1.0f));\n\
			}"
		);

		m_ShaderProgram = Toshi::TRenderSDL::CreateShaderProgram( m_VertexShader, m_PixelShader );
	}

	void StartFlush()
	{
		auto pShadowColor  = (Toshi::TVector4*)( *(TUINT*)( 0x0079a854 ) + 0xF0 );
		auto pAmbientColor = (Toshi::TVector4*)( *(TUINT*)( 0x0079a854 ) + 0x100 );

		auto pRenderContext = TSTATICCAST(
		    Toshi::TRenderContextD3D,
		    THookedRenderD3DInterface::GetSingleton()->GetCurrentContext()
		);

		m_VAO.SetAttribPointer( 0, 3, GL_FLOAT, sizeof( WorldVertex ), (void*)offsetof( WorldVertex, Position ) );
		m_VAO.SetAttribPointer( 1, 3, GL_FLOAT, sizeof( WorldVertex ), (void*)offsetof( WorldVertex, Normal ) );
		m_VAO.SetAttribPointer( 2, 3, GL_FLOAT, sizeof( WorldVertex ), (void*)offsetof( WorldVertex, Color ) );
		m_VAO.SetAttribPointer( 3, 2, GL_FLOAT, sizeof( WorldVertex ), (void*)offsetof( WorldVertex, UV ) );

		m_ShaderProgram.Use();
		m_ShaderProgram.SetUniform( "u_Color", Toshi::TVector4( 1.0f, 1.0f, 1.0f, 1.0f ) );
		m_ShaderProgram.SetUniform( "u_ShadowColor", *pShadowColor );
		m_ShaderProgram.SetUniform( "u_AmbientColor", *pAmbientColor );
		m_ShaderProgram.SetUniform( "u_Projection", *(Toshi::TMatrix44*)( TUINT( pRenderContext ) + 0x3C0 ) );

		glEnable( GL_CULL_FACE );
		glCullFace( GL_BACK );

		glEnable( GL_DEPTH_TEST );
		glPolygonMode( GL_BACK, GL_FILL );
	}

	void Render( Toshi::TRenderPacket* a_pRenderPacket )
	{
		auto pMesh       = a_pRenderPacket->GetMesh();
		auto pVertexPool = *TREINTERPRETCAST( Toshi::TVertexPoolResource**, TUINT( pMesh ) + 0x18 );
		auto pIndexPool  = *TREINTERPRETCAST( Toshi::TIndexPoolResource**, ( *TREINTERPRETCAST( TUINT*, TUINT( pMesh ) + 0x1C ) ) + 0x8 );

		{
			Toshi::TVertexBlockResource::HALBuffer vertexHAL;
			CALL_THIS( 0x006d6660, Toshi::TVertexPoolResource*, TBOOL, pVertexPool, Toshi::TVertexBlockResource::HALBuffer*, &vertexHAL );

			BYTE* pVertexData;
			vertexHAL.apVertexBuffers[ 0 ]->Lock( 0, 0, &pVertexData, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY );

			m_VAO.GetVertexBuffer().SetData( pVertexData + sizeof( WorldVertex ) * vertexHAL.uiVertexOffset, sizeof( WorldVertex ) * pVertexPool->GetNumVertices(), GL_DYNAMIC_DRAW );

			vertexHAL.apVertexBuffers[ 0 ]->Unlock();
		}

		{
			Toshi::TIndexBlockResource::HALBuffer indexHAL;
			CALL_THIS( 0x006d6180, Toshi::TIndexPoolResource*, TBOOL, pIndexPool, Toshi::TIndexBlockResource::HALBuffer*, &indexHAL );

			BYTE* pIndexData;
			indexHAL.pIndexBuffer->Lock( 0, 0, &pIndexData, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY );

			m_VAO.GetIndexBuffer().SetData( pIndexData + 2 * indexHAL.uiIndexOffset, 2 * pIndexPool->GetNumIndices(), GL_DYNAMIC_DRAW );

			indexHAL.pIndexBuffer->Unlock();
		}

		m_ShaderProgram.SetUniform( "u_View", a_pRenderPacket->GetModelViewMatrix() );

		m_VAO.Bind();
		glDrawElements( GL_TRIANGLE_STRIP, pIndexPool->GetNumIndices(), GL_UNSIGNED_SHORT, TNULL );
	}

private:
	Toshi::TGLVertexArrayRef m_VAO;
	Toshi::TGLShaderRef      m_VertexShader;
	Toshi::TGLShaderRef      m_PixelShader;
	Toshi::TGLShaderProgram  m_ShaderProgram;
};
