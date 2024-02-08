#include "pch.h"
#include <AImGUI.h>
#include <ModLoader.h>
#include <AModLoaderTask.h>
#include <AHooks.h>
#include <HookHelpers.h>
#include <BYardSDK/AGUI2.h>
#include <BYardSDK/THookedRenderD3DInterface.h>

#include <Toshi/THPTimer.h>
#include <File/TFile.h>
#include <Render/TIndexPoolResource.h>
#include <Render/TVertexPoolResource.h>
#include <Platform/DX8/TRenderContext_DX8.h>

#include "AWorldShaderGL.h"
#include "ASkinShaderGL.h"

#include "SOIL2/SOIL2.h"

TOSHI_NAMESPACE_USING

TBOOL g_bRenderGUI = TTRUE;
TBOOL g_bRenderWorldWireframe = TFALSE;
TBOOL g_bRenderSkinWireframe = TFALSE;

TBOOL MaterialLibrary_LoadTTLData(AMaterialLibrary* a_pMatLib, AMaterialLibrary::TTL* a_pTTLData)
{
	auto pTTL = TSTATICCAST(AMaterialLibrary::TTL*, a_pTTLData);

	auto pLibList = AMaterialLibraryManager::List::GetSingleton();
	TINT iNumTextures = 0;

	if (AMaterialLibrary::ms_bSkipLoadedTextures)
	{
		for (TINT i = 0; i < pTTL->m_iNumTextures; i++)
		{
			if (!pLibList->FindTexture(pTTL->m_pTextureInfos[i].m_szFileName, TNULL, TNULL))
			{
				iNumTextures++;
			}
		}
	}
	else
	{
		iNumTextures = pTTL->m_iNumTextures;
	}

	a_pMatLib->m_TexturesArray.Create(iNumTextures);
	a_pMatLib->m_pTextures = a_pMatLib->m_TexturesArray.GetArray();
	a_pMatLib->m_iNumTextures = iNumTextures;

	auto pGLTextureManager = TTextureManagerSDL::GetSingleton();

	for (TINT i = 0; i < iNumTextures; i++)
	{
		auto pTexInfo = &pTTL->m_pTextureInfos[i];

		if (!AMaterialLibrary::ms_bSkipLoadedTextures || !pLibList->FindTexture(pTexInfo->m_szFileName, TNULL, TNULL))
		{
			TASSERT(pTexInfo->m_bIsT2Texture == TRUE, "No support of other texture types");
			a_pMatLib->m_pTextures[i].Name = pTexInfo->m_szFileName;

			if (pTexInfo->m_bIsT2Texture == TRUE)
			{
				auto pTexture = new T2Texture;

				if (pTexture)
				{
					pTexture->SetData(pTexInfo->m_pData, pTexInfo->m_uiDataSize);
					
					auto pRenderSDL = TRenderSDL::Interface();
					TRenderInterface::SetSingletonExplicit(THookedRenderD3DInterface::GetSingleton());
					pTexture->Load();
					TRenderInterface::SetSingletonExplicit(pRenderSDL);

					pTexture->SetData(TNULL, 0);

					int width, height, channels;
					unsigned char* texData = SOIL_load_image_from_memory((unsigned char*)pTexInfo->m_pData, pTexInfo->m_uiDataSize, &width, &height, &channels, 4);

					glActiveTexture(GL_TEXTURE0);

					auto pGLTexture = TTextureSDL::InitRunTime(TEXTURE_FORMAT_R8G8B8A8_UNORM, width, height, texData);
					//pGLTexture->SetName(pTexInfo->m_szFileName);

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

					SOIL_free_image_data(texData);
					pGLTextureManager->AddGLAssociation(pTexture->GetD3DTexture(), pGLTexture);
				}

				a_pMatLib->m_pTextures[i].pTexture = pTexture;
			}
		}
	}

	return TTRUE;
}

MEMBER_HOOK(0x00615f60, AMaterialLibrary, AMaterialLibrary_DestroyTextures, void)
{
	auto pGLTextureManager = TTextureManagerSDL::GetSingleton();

	for (TINT i = 0; i < m_iNumTextures; i++)
	{
		auto pAppTexture = &m_pTextures[i];
		auto pGLTexture = pGLTextureManager->RemoveGLAssociation(pAppTexture->pTexture->GetD3DTexture());

		if (pGLTexture)
		{
			pGLTexture->Deinit();
		}
	}

	CallOriginal();
}

class ARenderer { };

MEMBER_HOOK(0x006154c0, ARenderer, ARenderer_CreateTRender, TBOOL)
{
	TRenderInterface::DISPLAYPARAMS displayParams;
	displayParams.bWindowed = TTRUE;
	displayParams.uiWidth = 800;
	displayParams.uiHeight = 600;
	displayParams.uiColourDepth = 32;

	auto pSDLRender = new TRenderSDL();
	pSDLRender->Create("Barnyard (OpenGL)");
	pSDLRender->CreateDisplay(displayParams);

	AWorldShaderGL::CreateSingleton();
	ASkinShaderGL::CreateSingleton();

	return CallOriginal();
}

class AGUI2RendererDX8 : public AGUI2Renderer
{
public:
	AGUI2Transform* m_pTransforms;
	TINT m_iTransformCount;
	TUINT32 m_uiColour;
	TBOOL m_bIsTransformDirty;
	AGUI2Material* m_pMaterial; // ?
};

MEMBER_HOOK(0x0064eb90, AGUI2RendererDX8, AGUI2Renderer_SetMaterial, void, AGUI2Material* a_pMaterial)
{
	if (m_pMaterial != a_pMaterial)
	{
		CallOriginal(a_pMaterial);

		if (a_pMaterial)
		{
			auto pTextureResource = TSTATICCAST(TTextureResourceHAL*, a_pMaterial->m_pTextureResource);

			if (pTextureResource)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, (GLuint)TTextureManagerSDL::GetSingleton()->GetGLAssociation(pTextureResource->GetD3DTexture())->GetHandle());
			}
		}
		else
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glCullFace(GL_FRONT);
	glDisable(GL_DEPTH_TEST);
}

struct AGUI2RendererVertex
{
	Toshi::TVector3 Position;
	TUINT32 Colour;
	Toshi::TVector2 UV;
};

MEMBER_HOOK(0x0064fa70, AGUI2RendererDX8, AGUI2Renderer_RenderRectangle, void, const Toshi::TVector2& a, const Toshi::TVector2& b, const Toshi::TVector2& uv1, const Toshi::TVector2& uv2)
{
	CallOriginal(a, b, uv1, uv2);

	if (!g_bRenderGUI) return;

	TFLOAT fZ = *(TFLOAT*)0x00779938 * -1.0f;

	static AGUI2RendererVertex s_Vertices[8];
	s_Vertices[0].Position.x = a.x;
	s_Vertices[0].Position.y = a.y;
	s_Vertices[0].Position.z = fZ;
	s_Vertices[0].Colour = m_uiColour;
	s_Vertices[0].UV.x = uv1.x;
	s_Vertices[0].UV.y = uv1.y;
	s_Vertices[1].Position.y = a.y;
	s_Vertices[1].Position.x = b.x;
	s_Vertices[1].Position.z = fZ;
	s_Vertices[1].Colour = m_uiColour;
	s_Vertices[1].UV.y = uv1.y;
	s_Vertices[1].UV.x = uv2.x;
	s_Vertices[2].Position.y = b.y;
	s_Vertices[2].Position.x = a.x;
	s_Vertices[2].Position.z = fZ;
	s_Vertices[2].Colour = m_uiColour;
	s_Vertices[2].UV.y = uv2.y;
	s_Vertices[2].UV.x = uv1.x;
	s_Vertices[3].Position.y = b.y;
	s_Vertices[3].Position.x = b.x;
	s_Vertices[3].Position.z = fZ;
	s_Vertices[3].Colour = m_uiColour;
	s_Vertices[3].UV.y = uv2.y;
	s_Vertices[3].UV.x = uv2.x;

	static TUINT16 s_Indices[] = {
		0, 1, 2, 3
	};

	static auto vertexShader = TRenderSDL::CompileShader(
		GL_VERTEX_SHADER,
		"#version 400\n\
		\
		layout(location = 0) in vec3 a_Position;\n\
		layout(location = 1) in vec4 a_Colour;\n\
		layout(location = 2) in vec2 a_UV;\n\
		\
		out vec4 o_Colour;\n\
		out vec2 o_TexCoord;\n\
		\
		uniform mat4 u_Projection;\n\
		uniform mat4 u_View;\n\
		\
		void main()\n\
		{\n\
			o_TexCoord = a_UV;\n\
			o_Colour = vec4(a_Colour.b, a_Colour.g, a_Colour.r, a_Colour.a);\n\
			gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);\n\
		}"
	);

	static auto fragmentShader = TRenderSDL::CompileShader(
		GL_FRAGMENT_SHADER,
		"#version 400\n\
		\
		out vec4 color;\n\
		\
		in vec4 o_Colour;\n\
		in vec2 o_TexCoord;\n\
		\
		uniform sampler2D tex0;\n\
		\
		void main()\n\
		{\n\
			vec4 texColor = texture(tex0, o_TexCoord);\n\
			if (texColor.a < 0.2f) discard;\n\
			color = texture(tex0, o_TexCoord) * o_Colour;\n\
		}"
	);

	static auto shaderProgram = Toshi::TRenderSDL::CreateShaderProgram(vertexShader, fragmentShader);

	static auto s_VertexArray = TRenderSDL::CreateVertexArray(
		TRenderSDL::CreateVertexBuffer(s_Vertices, sizeof(s_Vertices), GL_DYNAMIC_DRAW),
		TRenderSDL::CreateIndexBuffer(s_Indices, 4, GL_STATIC_DRAW)
	);

	s_VertexArray.SetAttribPointer(0, 3, GL_FLOAT, sizeof(AGUI2RendererVertex), 0);
	s_VertexArray.SetAttribPointer(1, 4, GL_UNSIGNED_BYTE, sizeof(AGUI2RendererVertex), (void*)offsetof(AGUI2RendererVertex, Colour), GL_TRUE);
	s_VertexArray.SetAttribPointer(2, 2, GL_FLOAT, sizeof(AGUI2RendererVertex), (void*)offsetof(AGUI2RendererVertex, UV));

	s_VertexArray.GetVertexBuffer().SetData(&s_Vertices, sizeof(AGUI2RendererVertex) * 4, GL_DYNAMIC_DRAW);

	auto pTransform = m_pTransforms + m_iTransformCount;

	TMatrix44 projection = {
		2.0f / 800.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f / 600.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	TMatrix44 view;
	view.m_f11 = pTransform->m_Rotation[0].x;
	view.m_f12 = pTransform->m_Rotation[0].y;
	view.m_f13 = 0.0f;
	view.m_f14 = 0.0f;
	view.m_f21 = pTransform->m_Rotation[1].x;
	view.m_f22 = pTransform->m_Rotation[1].y;
	view.m_f23 = 0.0f;
	view.m_f24 = 0.0f;
	view.m_f31 = 0.0f;
	view.m_f32 = 0.0f;
	view.m_f33 = 1.0f;
	view.m_f34 = 0.0f;
	view.m_f41 = pTransform->m_Position.x;
	view.m_f42 = pTransform->m_Position.y;
	view.m_f43 = 0.0f;
	view.m_f44 = 1.0f;

	shaderProgram.Use();
	shaderProgram.SetUniform("u_Projection", projection);
	shaderProgram.SetUniform("u_View", view);

	s_VertexArray.Bind();
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, TNULL);
}

MEMBER_HOOK(0x005f6f70, AWorldMaterialWrapperGL, AWorldMaterial_PreRender, void)
{
	CallOriginal();
	AWorldMaterialWrapperGL::PreRenderGL();
}

class AWorldShaderHAL { };

MEMBER_HOOK(0x005f6510, AWorldShaderHAL, AWorldShaderHAL_StartFlush, void)
{
	CallOriginal();
	AWorldShaderGL::GetSingleton()->StartFlush();
}

MEMBER_HOOK(0x005f6cb0, AWorldShaderHAL, AWorldShaderHAL_Render, void, Toshi::TRenderPacket* a_pRenderPacket)
{
	CallOriginal(a_pRenderPacket);

	if (*(TBOOL*)(TUINT(this) + 0xEC) == TFALSE)
	{
		return;
	}

	AWorldShaderGL::GetSingleton()->Render(a_pRenderPacket);
}

MEMBER_HOOK(0x005f3ba0, ASkinMaterialWrapperGL, ASkinMaterial_PreRender, void)
{
	CallOriginal();
	ASkinMaterialWrapperGL::PreRenderGL();
}

class ASkinShaderHAL { };

MEMBER_HOOK(0x005f3230, ASkinShaderHAL, ASkinShader_StartFlush, void)
{
	CallOriginal();
	ASkinShaderGL::GetSingleton()->StartFlush();
}

MEMBER_HOOK(0x005f4830, ASkinShaderHAL, ASkinShader_Render, void, Toshi::TRenderPacket* a_pRenderPacket)
{
	CallOriginal(a_pRenderPacket);
	ASkinShaderGL::GetSingleton()->Render(a_pRenderPacket);
}

MEMBER_HOOK(0x006c6590, TRenderD3DInterface, TRenderD3DInterface_BeginScene, TBOOL)
{
	if (CallOriginal())
	{
		return TRenderSDL::Interface()->BeginScene();
	}

	return TFALSE;
}

MEMBER_HOOK(0x006c6fd0, TRenderD3DInterface, TRenderD3DInterface_EndScene, TBOOL)
{
	if (CallOriginal())
	{
		return TRenderSDL::Interface()->EndScene();
	}

	return TFALSE;
}

MEMBER_HOOK(0x006be990, TRenderD3DInterface, TRenderD3DInterface_FlushShaders, void)
{
	CallOriginal();
	TRenderSDL::Interface()->FlushShaders();
}

class ABYGLRender : public AModInstance
{
public:
	TBOOL OnLoad() override
	{
		AHooks::AddHook(Hook_MaterialLibrary_LoadTTLData, HookType_Before, MaterialLibrary_LoadTTLData);
		InstallHook<ARenderer_CreateTRender>();
		InstallHook<TRenderD3DInterface_BeginScene>();
		InstallHook<TRenderD3DInterface_EndScene>();
		InstallHook<TRenderD3DInterface_FlushShaders>();
		InstallHook<AGUI2Renderer_SetMaterial>();
		InstallHook<AGUI2Renderer_RenderRectangle>();
		InstallHook<AWorldMaterial_PreRender>();
		InstallHook<AWorldShaderHAL_Render>();
		InstallHook<AWorldShaderHAL_StartFlush>();
		InstallHook<ASkinMaterial_PreRender>();
		InstallHook<ASkinShader_Render>();
		InstallHook<ASkinShader_StartFlush>();
		InstallHook<AMaterialLibrary_DestroyTextures>();
		return TTRUE;
	}

	TBOOL OnUpdate(TFLOAT a_fDeltaTime) override
	{
		return TTRUE;
	}

	void OnUnload() override
	{

	}

	void OnRenderInterfaceReady(Toshi::TRenderD3DInterface* a_pRenderInterface) override
	{
		//THookedRenderD3DInterface::GetSingleton();
	}

	void OnAppRendererReady() override
	{

	}

	virtual void OnImGuiRender() override
	{ 
		ImGui::Checkbox("Render GUI", &g_bRenderGUI);
		ImGui::Checkbox("Wireframe World", &g_bRenderWorldWireframe);
		ImGui::Checkbox("Wireframe Skin", &g_bRenderSkinWireframe);
	}

	virtual TBOOL HasSettingsUI() override
	{ 
		return TTRUE;
	}

	const char* GetName() override
	{
		return "BYGLRender";
	}
};

extern "C"
{
	MODLOADER_EXPORT AModInstance* CreateModInstance()
	{
		TMemory::Initialise(2 * 1024 * 1024, 0);

		TUtil::ToshiCreate(
			TUtil::TOSHIParams{
				.szCommandLine = "",
				.szLogFileName = "glrender",
				.szLogAppName = "BYGLRender"
			}
		);

		return new ABYGLRender();
	}
}
