#pragma once
#include "TRender_SDL.h"
#include "TTexture_SDL.h"

#include <Render/TTexture.h>
#include <Platform/DX8/TTextureResourceHAL_DX8.h>

class ASkinMaterialWrapperGL :
    public Toshi::TMaterial
{
public:
	void PreRenderGL()
	{
		if ( m_pTexture )
		{
			auto pD3DTexture = m_pTexture->GetD3DTexture();

			if ( !m_pTextures[ 0 ] || pD3DTexture != (IDirect3DTexture8*)m_pTextures[ 1 ] )
			{
				auto pInvalidNative = THookedRenderD3DInterface::GetSingleton()->GetInvalidTexture();

				if ( m_pTexture != pInvalidNative )
				{
					auto pTexManagerSDL = Toshi::TTextureManagerSDL::GetSingleton();
					m_pTextures[ 0 ]    = TREINTERPRETCAST( Toshi::TTexture*, pTexManagerSDL->GetGLAssociation( pD3DTexture ) );
					m_pTextures[ 1 ]    = TREINTERPRETCAST( Toshi::TTexture*, pD3DTexture );

					if ( m_pLighting1 )
					{
						m_pTextures[ 2 ] = TREINTERPRETCAST( Toshi::TTexture*, pTexManagerSDL->GetGLAssociation( m_pLighting1->GetD3DTexture() ) );
					}

					if ( m_pLighting2 )
					{
						m_pTextures[ 3 ] = TREINTERPRETCAST( Toshi::TTexture*, pTexManagerSDL->GetGLAssociation( m_pLighting2->GetD3DTexture() ) );
					}

					if ( m_pLighting3 )
					{
						m_pTextures[ 4 ] = TREINTERPRETCAST( Toshi::TTexture*, pTexManagerSDL->GetGLAssociation( m_pLighting3->GetD3DTexture() ) );
					}

					if ( m_pLighting4 )
					{
						m_pTextures[ 5 ] = TREINTERPRETCAST( Toshi::TTexture*, pTexManagerSDL->GetGLAssociation( m_pLighting4->GetD3DTexture() ) );
					}
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

		if ( ( ( m_iBlendMode == 0 ) || ( m_iBlendMode == 1 ) ) || ( m_iBlendMode != 3 ) )
		{
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			glDepthMask( GL_TRUE );
		}
		else
		{
			glBlendFunc( GL_SRC_ALPHA, GL_ONE );
			glDepthMask( GL_FALSE );
		}
	}

public:
	Toshi::TTextureResourceHAL* m_pTexture;
	Toshi::TTextureResourceHAL* m_pLighting1;
	Toshi::TTextureResourceHAL* m_pLighting2;
	Toshi::TTextureResourceHAL* m_pLighting3;
	Toshi::TTextureResourceHAL* m_pLighting4;
	TINT                        m_iBlendMode;
	TINT                        UNK1;
	TINT                        UNK2;
	TINT                        UNK3;
	TBOOL                       m_bIsSkin; // ?
	TBOOL                       m_bFlag2;
	TBOOL                       m_bFlag3;
};

class ASkinShaderGL :
    public Toshi::TSingleton<ASkinShaderGL>
{
public:
	struct SkinVertex
	{
		Toshi::TVector3 Position;
		Toshi::TVector3 Normal;
		TUINT8          Weights[ 4 ];
		TUINT8          Bones[ 4 ];
		Toshi::TVector2 UV;
	};

public:
	ASkinShaderGL()
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
			layout(location = 2) in vec4 a_Weights;\n\
			layout(location = 3) in vec4 a_Bones;\n\
			layout(location = 4) in vec2 a_UV;\n\
			\
			out vec2 o_TexCoord;\n\
			out vec3 o_Normal;\n\
			out vec3 o_Position;\n\
			out vec3 o_ViewPos;\n\
			out vec4 o_FinalColor;\n\
			\
			uniform vec4 u_AmbientColour;\n\
			uniform vec4 u_LightColour;\n\
			uniform vec3 u_LightDirection;\n\
			uniform vec4 u_UpAxis;\n\
			uniform vec4 u_LightColourParams;\n\
			uniform mat4 u_Projection;\n\
			uniform mat4 u_ModelView;\n\
			uniform mat4 u_Model;\n\
			uniform mat4 u_ViewWorld;\n\
			uniform mat4 u_BoneTransforms[28];\n\
			uniform int u_NumBones;\n\
			\
			void main()\n\
			{\n\
				vec3 vertex = vec3(0.0f);\n\
				vec3 normal = vec3(0.0f);\n\
				for (int i = 0; i < 4; i++)\n\
				{\n\
					int boneIndex = int(a_Bones[i] * 256.0 / 3);\n\
					vertex += vec3((vec4(a_Position, 1.0f) * u_BoneTransforms[boneIndex]) * a_Weights[i]);\n\
					normal += (a_Normal * mat3(u_BoneTransforms[boneIndex])) * a_Weights[i];\n\
				}\n\
				gl_Position = (u_Projection * u_ModelView) * vec4(vertex, 1.0f);\n\
				o_Position = mat3(u_Model) * vertex;\n\
				o_Normal = normalize(normal);\n\
				o_ViewPos = mat3(u_ViewWorld) * vec3(0.0, 0.0, 0.0);\n\
				o_TexCoord = a_UV;\n\
				float diffuse = min(max(dot(o_Normal, -u_LightDirection), 0.0f), 1.0f);\n\
				vec4 finalColor = diffuse * u_LightColour;\n\
				finalColor += (1.0f - diffuse) * u_AmbientColour;\n\
				o_FinalColor.rgb = finalColor.rgb;\n\
				o_FinalColor.a = u_AmbientColour.a;\n\
			}"
		);

		m_PixelShader = Toshi::TRenderSDL::CompileShader(
		    GL_FRAGMENT_SHADER,
		    "#version 400\n\
			in vec2 o_TexCoord;\n\
			in vec3 o_Normal;\n\
			in vec3 o_Position;\n\
			in vec3 o_ViewPos;\n\
			in vec4 o_FinalColor;\n\
			\
			out vec4 color;\n\
			\
			uniform sampler2D tex0;\n\
			\
			void main()\n\
			{\n\
				vec4 texColor = texture(tex0, o_TexCoord);\n\
				if (texColor.a < 0.2f) discard;\n\
				color = texColor * clamp(o_FinalColor, 0.0f, 1.0f);\n\
			}"
		);

		m_ShaderProgram = Toshi::TRenderSDL::CreateShaderProgram( m_VertexShader, m_PixelShader );

		m_VertexShaderHD = Toshi::TRenderSDL::CompileShader(
		    GL_VERTEX_SHADER,
		    "#version 400\n\
			\
			layout(location = 0) in vec3 a_Position;\n\
			layout(location = 1) in vec3 a_Normal;\n\
			layout(location = 2) in vec4 a_Weights;\n\
			layout(location = 3) in vec4 a_Bones;\n\
			layout(location = 4) in vec2 a_UV;\n\
			\
			out vec2 o_TexCoord;\n\
			out vec3 o_Normal;\n\
			out vec3 o_Position;\n\
			out vec3 o_ViewPos;\n\
			out vec2 o_LightingTexCoord0;\n\
			out vec2 o_LightingTexCoord1;\n\
			out vec2 o_LightingTexCoord2;\n\
			out vec2 o_LightingTexCoord3;\n\
			\
			uniform vec3 u_LightDirection;\n\
			uniform vec4 u_UpAxis;\n\
			uniform vec4 u_LightColourParams;\n\
			uniform mat4 u_Projection;\n\
			uniform mat4 u_ModelView;\n\
			uniform mat4 u_Model;\n\
			uniform mat4 u_ViewWorld;\n\
			uniform mat4 u_BoneTransforms[28];\n\
			uniform int u_NumBones;\n\
			\
			void main()\n\
			{\n\
				vec3 vertex = vec3(0.0f);\n\
				vec3 normal = vec3(0.0f);\n\
				for (int i = 0; i < 4; i++)\n\
				{\n\
					int boneIndex = int(a_Bones[i] * 256.0 / 3);\n\
					vertex += vec3((vec4(a_Position, 1.0f) * u_BoneTransforms[boneIndex]) * a_Weights[i]);\n\
					normal += (a_Normal * mat3(u_BoneTransforms[boneIndex])) * a_Weights[i];\n\
				}\n\
				gl_Position = (u_Projection * u_ModelView) * vec4(vertex, 1.0f);\n\
				o_Position = mat3(u_Model) * vertex;\n\
				o_Normal = normalize(normal);\n\
				o_ViewPos = mat3(u_ViewWorld) * vec3(0.0, 0.0, 0.0);\n\
				o_TexCoord = a_UV;\n\
				vec2 lightingUV;\n\
				lightingUV.x = dot(o_Normal, -u_LightDirection);\n\
				lightingUV.y = dot(o_Normal, vec3(u_UpAxis));\n\
				o_LightingTexCoord0 = lightingUV;\n\
				o_LightingTexCoord1 = lightingUV;\n\
				o_LightingTexCoord2 = lightingUV;\n\
				o_LightingTexCoord3 = lightingUV;\n\
			}"
		);

		m_PixelShaderHD = Toshi::TRenderSDL::CompileShader(
		    GL_FRAGMENT_SHADER,
		    "#version 400\n\
			in vec2 o_TexCoord;\n\
			in vec3 o_Normal;\n\
			in vec3 o_Position;\n\
			in vec3 o_ViewPos;\n\
			in vec2 o_LightingTexCoord0;\n\
			in vec2 o_LightingTexCoord1;\n\
			in vec2 o_LightingTexCoord2;\n\
			in vec2 o_LightingTexCoord3;\n\
			\
			out vec4 color;\n\
			\
			uniform vec4 u_AmbientColour;\n\
			uniform vec4 u_LightColour;\n\
			uniform vec3 u_LightDirection;\n\
			uniform sampler2D tex0;\n\
			uniform sampler2D Lighting1;\n\
			uniform sampler2D Lighting2;\n\
			uniform sampler2D Lighting3;\n\
			uniform sampler2D Lighting4;\n\
			uniform vec4 u_LightingLerp1;\n\
			uniform vec4 u_LightingLerp2;\n\
			\
			void main()\n\
			{\n\
				color = texture(tex0, o_TexCoord);\n\
				vec4 lighting1Color = texture(Lighting1, o_LightingTexCoord0);\n\
				vec4 lighting2Color = texture(Lighting2, o_LightingTexCoord1);\n\
				vec4 lighting3Color = texture(Lighting3, o_LightingTexCoord2);\n\
				vec4 lighting4Color = texture(Lighting4, o_LightingTexCoord3);\n\
				vec4 shade1 = mix(lighting3Color, lighting1Color, u_LightingLerp1);\n\
				vec4 shade2 = mix(lighting4Color, lighting2Color, u_LightingLerp1);\n\
				color.rgb = clamp(color.rgb - shade1.rgb, 0.0f, 1.0f);\n\
				color.rgb = clamp(color.rgb + shade2.rgb, 0.0f, 1.0f);\n\
				color.a *= u_AmbientColour.a;\n\
			}"
		);

		m_ShaderProgramHD = Toshi::TRenderSDL::CreateShaderProgram( m_VertexShaderHD, m_PixelShaderHD );
	}

	void StartFlush()
	{
		glEnable( GL_CULL_FACE );
		glCullFace( GL_BACK );

		glEnable( GL_DEPTH_TEST );
		glPolygonMode( GL_BACK, GL_FILL );

		auto pRenderContext = TSTATICCAST(
		    Toshi::TRenderContextD3D,
		    THookedRenderD3DInterface::GetSingleton()->GetCurrentContext()
		);

		m_ProjectionMatrix = *(Toshi::TMatrix44*)( TUINT( pRenderContext ) + 0x3C0 );
		m_WorldViewMatrix  = *(Toshi::TMatrix44*)( TUINT( pRenderContext ) + 0x8C );
		m_ViewWorldMatrix.Invert( m_WorldViewMatrix );
	}

	void Render( Toshi::TRenderPacket* a_pRenderPacket )
	{
		auto pMesh       = a_pRenderPacket->GetMesh();
		auto pVertexPool = *TREINTERPRETCAST( Toshi::TVertexPoolResource**, TUINT( pMesh ) + 0x18 );

		auto pMaterial = TSTATICCAST( ASkinMaterialWrapperGL, pMesh->GetMaterial() );

		Toshi::TGLShaderProgram* pShaderProgram = TNULL;

		if ( pMaterial->m_bIsSkin && pMaterial->m_bFlag2 && pMaterial->m_bFlag3 )
		{
			m_ShaderProgramHD.Use();
			pShaderProgram = &m_ShaderProgramHD;

			Toshi::TMatrix44 inverseModelView;
			inverseModelView.InvertOrthogonal( a_pRenderPacket->GetModelViewMatrix() );

			Toshi::TVector4 upVector;
			upVector.Negate3( inverseModelView.AsBasisVector4( 2 ) );

			pShaderProgram->SetUniform( "u_UpAxis", upVector );

			{
				auto pGLTexture = TREINTERPRETCAST( Toshi::TTextureSDL*, pMaterial->GetTexture( 2 ) );
				pShaderProgram->SetUniform( "Lighting1", 1 );
				glActiveTexture( GL_TEXTURE1 );
				glBindTexture( GL_TEXTURE_2D, (GLuint)pGLTexture->GetHandle() );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			}

			{
				auto pGLTexture = TREINTERPRETCAST( Toshi::TTextureSDL*, pMaterial->GetTexture( 3 ) );
				pShaderProgram->SetUniform( "Lighting2", 2 );
				glActiveTexture( GL_TEXTURE2 );
				glBindTexture( GL_TEXTURE_2D, (GLuint)pGLTexture->GetHandle() );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			}

			{
				auto pGLTexture = TREINTERPRETCAST( Toshi::TTextureSDL*, pMaterial->GetTexture( 4 ) );
				pShaderProgram->SetUniform( "Lighting3", 3 );
				glActiveTexture( GL_TEXTURE3 );
				glBindTexture( GL_TEXTURE_2D, (GLuint)pGLTexture->GetHandle() );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			}

			{
				auto pGLTexture = TREINTERPRETCAST( Toshi::TTextureSDL*, pMaterial->GetTexture( 5 ) );
				pShaderProgram->SetUniform( "Lighting4", 4 );
				glActiveTexture( GL_TEXTURE4 );
				glBindTexture( GL_TEXTURE_2D, (GLuint)pGLTexture->GetHandle() );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			}

			TFLOAT fLerpFactor = a_pRenderPacket->GetShadeCoeff() * ( 1.0f / 255.0f );

			pShaderProgram->SetUniform( "u_LightingLerp1", Toshi::TVector4( 1.0f - fLerpFactor, 1.0f - fLerpFactor, 1.0f - fLerpFactor, 0.0f ) );
			pShaderProgram->SetUniform( "u_LightingLerp2", Toshi::TVector4( fLerpFactor, fLerpFactor, fLerpFactor, 0.0f ) );
		}
		else
		{
			m_ShaderProgram.Use();
			pShaderProgram = &m_ShaderProgram;

			Toshi::TVector4 upVector = { 0.0f, 0.0f, 0.0f, 1.0f };
			pShaderProgram->SetUniform( "u_UpAxis", upVector );

			// TODO: Use simple shader instead of HD...
		}

		pShaderProgram->SetUniform( "u_Projection", m_ProjectionMatrix );
		pShaderProgram->SetUniform( "u_ViewWorld", m_ViewWorldMatrix );

		m_VAO.Bind();

		Toshi::TVertexBlockResource::HALBuffer vertexHAL;
		CALL_THIS( 0x006d6660, Toshi::TVertexPoolResource*, TBOOL, pVertexPool, Toshi::TVertexBlockResource::HALBuffer*, &vertexHAL );

		BYTE* pVertexData;
		vertexHAL.apVertexBuffers[ 0 ]->Lock( 0, 0, &pVertexData, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY );

		m_VAO.SetAttribPointer( 0, 3, GL_FLOAT, sizeof( SkinVertex ), (void*)offsetof( SkinVertex, Position ) );
		m_VAO.SetAttribPointer( 1, 3, GL_FLOAT, sizeof( SkinVertex ), (void*)offsetof( SkinVertex, Normal ) );
		m_VAO.SetAttribPointer( 2, 4, GL_UNSIGNED_BYTE, sizeof( SkinVertex ), (void*)offsetof( SkinVertex, Weights ), GL_TRUE );
		m_VAO.SetAttribPointer( 3, 4, GL_UNSIGNED_BYTE, sizeof( SkinVertex ), (void*)offsetof( SkinVertex, Bones ), GL_TRUE );
		m_VAO.SetAttribPointer( 4, 2, GL_FLOAT, sizeof( SkinVertex ), (void*)offsetof( SkinVertex, UV ) );
		m_VAO.GetVertexBuffer().SetData( pVertexData + sizeof( SkinVertex ) * vertexHAL.uiVertexOffset, sizeof( SkinVertex ) * pVertexPool->GetNumVertices(), GL_DYNAMIC_DRAW );

		vertexHAL.apVertexBuffers[ 0 ]->Unlock();

		Toshi::TMatrix44 modelMatrix;
		modelMatrix.Multiply( m_ViewWorldMatrix, a_pRenderPacket->GetModelViewMatrix() );

		auto&            modelView = a_pRenderPacket->GetModelViewMatrix();
		Toshi::TMatrix44 worldModelView;

		for ( TINT i = 0; i < 4; i++ )
		{
			worldModelView.AsBasisVector4( i ).x = modelView.m_f11 * m_WorldViewMatrix.AsBasisVector3( i ).x + modelView.m_f12 * m_WorldViewMatrix.AsBasisVector3( i ).y + modelView.m_f13 * m_WorldViewMatrix.AsBasisVector3( i ).z;
			worldModelView.AsBasisVector4( i ).y = modelView.m_f21 * m_WorldViewMatrix.AsBasisVector3( i ).x + modelView.m_f22 * m_WorldViewMatrix.AsBasisVector3( i ).y + modelView.m_f23 * m_WorldViewMatrix.AsBasisVector3( i ).z;
			worldModelView.AsBasisVector4( i ).z = modelView.m_f31 * m_WorldViewMatrix.AsBasisVector3( i ).x + modelView.m_f32 * m_WorldViewMatrix.AsBasisVector3( i ).y + modelView.m_f33 * m_WorldViewMatrix.AsBasisVector3( i ).z;
			worldModelView.AsBasisVector4( i ).w = modelView.m_f14 * m_WorldViewMatrix.AsBasisVector3( i ).x + modelView.m_f24 * m_WorldViewMatrix.AsBasisVector3( i ).y + modelView.m_f34 * m_WorldViewMatrix.AsBasisVector3( i ).z;
		};

		auto lightDirX = -a_pRenderPacket->GetLightDirection().x;
		auto lightDirY = -a_pRenderPacket->GetLightDirection().y;
		auto lightDirZ = -a_pRenderPacket->GetLightDirection().z;

		Toshi::TVector3 lightDirWorld;
		lightDirWorld.x = worldModelView.m_f11 * lightDirX + worldModelView.m_f21 * lightDirY + worldModelView.m_f31 * lightDirZ;
		lightDirWorld.y = worldModelView.m_f12 * lightDirX + worldModelView.m_f22 * lightDirY + worldModelView.m_f32 * lightDirZ;
		lightDirWorld.z = worldModelView.m_f13 * lightDirX + worldModelView.m_f23 * lightDirY + worldModelView.m_f33 * lightDirZ;
		lightDirWorld.Normalize();

		Toshi::TVector4 ambientColour = a_pRenderPacket->GetAmbientColour();
		Toshi::TVector4 lightColour   = a_pRenderPacket->GetLightColour();

		auto pSkeletonInstance = a_pRenderPacket->GetSkeletonInstance();

		pShaderProgram->SetUniform( "u_Model", modelMatrix );
		pShaderProgram->SetUniform( "u_AmbientColour", ambientColour );
		pShaderProgram->SetUniform( "u_LightDirection", lightDirWorld );
		pShaderProgram->SetUniform( "u_LightColour", Toshi::TVector4( 1.0f, 1.0f, 1.0f, 1.0f ) );
		pShaderProgram->SetUniform( "u_LightColourParams", Toshi::TVector4( 1.0f, 0.0f, 0.0f, 1.0f ) );
		pShaderProgram->SetUniform( "u_ModelView", a_pRenderPacket->GetModelViewMatrix() );

		auto uiNumSubMeshes = *(TUINT16*)( TUINT( pMesh ) + 0x16 );

		for ( TUINT16 i = 0; i < uiNumSubMeshes; i++ )
		{
			auto pSubMesh   = ( *TREINTERPRETCAST( TUINT*, TUINT( pMesh ) + 0x1C ) ) + 0x94 * i;
			auto pIndexPool = *TREINTERPRETCAST( Toshi::TIndexPoolResource**, TUINT( pSubMesh ) + 0x4 );

			Toshi::TIndexBlockResource::HALBuffer indexHAL;
			CALL_THIS( 0x006d6180, Toshi::TIndexPoolResource*, TBOOL, pIndexPool, Toshi::TIndexBlockResource::HALBuffer*, &indexHAL );

			BYTE* pIndexData;
			indexHAL.pIndexBuffer->Lock( 0, 0, &pIndexData, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY );
			m_VAO.GetIndexBuffer().SetData( pIndexData + 2 * indexHAL.uiIndexOffset, 2 * pIndexPool->GetNumIndices(), GL_DYNAMIC_DRAW );
			indexHAL.pIndexBuffer->Unlock();

			// Update bones
			auto iSubMeshNumBones = *TREINTERPRETCAST( TINT32*, TUINT( pSubMesh ) + 0x20 );
			auto pSubMeshBones    = TREINTERPRETCAST( TUINT32*, TUINT( pSubMesh ) + 0x24 );

			static Toshi::TMatrix44 s_BoneTransforms[ 28 ];
			for ( TINT k = 0; k < iSubMeshNumBones; k++ )
			{
				D3DXMatrixTranspose( s_BoneTransforms[ k ], pSkeletonInstance->GetBone( pSubMeshBones[ k ] ).m_Transform );
			}

			pShaderProgram->SetUniform( "u_BoneTransforms", s_BoneTransforms, iSubMeshNumBones );
			pShaderProgram->SetUniform( "u_NumBones", iSubMeshNumBones );

			// Draw
			glDrawElements( GL_TRIANGLE_STRIP, pIndexPool->GetNumIndices(), GL_UNSIGNED_SHORT, TNULL );
		}
	}

private:
	Toshi::TGLVertexArrayRef m_VAO;

	// Usual shader
	Toshi::TGLShaderRef     m_VertexShader;
	Toshi::TGLShaderRef     m_PixelShader;
	Toshi::TGLShaderProgram m_ShaderProgram;

	// Shader with lighting
	Toshi::TGLShaderRef     m_VertexShaderHD;
	Toshi::TGLShaderRef     m_PixelShaderHD;
	Toshi::TGLShaderProgram m_ShaderProgramHD;

	Toshi::TMatrix44 m_WorldViewMatrix;
	Toshi::TMatrix44 m_ViewWorldMatrix;
	Toshi::TMatrix44 m_ProjectionMatrix;
};
