#include "pch.h"
#include "AImGuiRenderer.h"
#include "ImGui/AImGui.h"

AImGuiRenderer::AImGuiRenderer()
{
}

TBOOL AImGuiRenderer::CreateTRender()
{
	auto pRender = new Toshi::TRENDER();
	TBOOL bCreatedRender = pRender->Create();

	Toshi::TRender::DisplayParams displayParams;
	displayParams.Unk3 = 32;
	displayParams.Unk4 = 3;
	displayParams.Unk5 = TFALSE;
	displayParams.IsFullscreen = TFALSE;
	displayParams.MultisampleQualityLevel = 1;
	displayParams.Width = 1280;
	displayParams.Height = 720;
	pRender->CreateDisplay(&displayParams);
	
	return bCreatedRender && CreateMainViewport() && CreateImGui();
}

TBOOL AImGuiRenderer::CreateMainViewport()
{
	auto pRender = Toshi::TRENDER::Interface();
	auto pDisplayParams = pRender->GetCurrentDisplayParams();

	m_pViewport = new Toshi::TViewport;
	m_pViewport->SetX(0.0f);
	m_pViewport->SetY(0.0f);
	m_pViewport->SetWidth((float)pDisplayParams->Width);
	m_pViewport->SetHeight((float)pDisplayParams->Height);
	m_pViewport->SetMinZ(0.2f);
	m_pViewport->SetMaxZ(1.0f);
	m_pViewport->AllowBackgroundClear(TTRUE);
	m_pViewport->AllowDepthClear(TTRUE);
	m_pViewport->Enable(TTRUE);
	m_pViewport->EnableDefaultBeginRender(TTRUE);
	m_pViewport->SetBackgroundColor(255, 255, 255, 255);

	return TTRUE;
}

TBOOL AImGuiRenderer::CreateImGui()
{
	static float s_Vertices[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.0f, 0.5f, 0.0f,
	};

	static uint16_t s_Indices[] = { 0, 1, 2 };

	m_VertexArray = Toshi::TRenderSDL::CreateVertexArray(
		Toshi::TRenderSDL::CreateVertexBuffer(s_Vertices, sizeof(s_Vertices), GL_STATIC_DRAW),
		Toshi::TRenderSDL::CreateIndexBuffer(s_Indices, 3, GL_STATIC_DRAW)
	);

	m_VertexArray.SetAttribPointer(0, 3, GL_FLOAT, 12, 0);
	
	auto vertexShader = Toshi::TRenderSDL::CompileShader(
		GL_VERTEX_SHADER,
		"#version 400\n\
		\
		layout(location = 0) in vec3 a_Position;\n\
		\
		void main()\n\
		{\n\
			gl_Position = vec4(a_Position, 1.0);\n\
		}"
	);

	auto fragmentShader = Toshi::TRenderSDL::CompileShader(
		GL_FRAGMENT_SHADER,
		"#version 400\n\
		\
		out vec4 color;\n\
		\
		uniform vec4 u_Color;\n\
		\
		void main()\n\
		{\n\
			color = u_Color;\n\
		}"
	);

	m_ShaderProgram = Toshi::TRenderSDL::CreateShaderProgram(vertexShader, fragmentShader);
	
	m_ShaderProgram.Use();
	m_ShaderProgram.SetUniform("u_Color", Toshi::TVector4(1.0, 0.0, 0.0, 1.0));

	return AImGui::CreateSingleton() != TNULL;
}

void AImGuiRenderer::Update(float a_fDeltaTime)
{
	auto pRender = Toshi::TRENDER::Interface();

	pRender->Update(a_fDeltaTime);
	pRender->BeginScene();

	m_pViewport->Begin();
	
	m_VertexArray.Bind();
	m_ShaderProgram.Use();
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, TNULL);

	// Draw UI
	AImGui::GetSingleton()->Render();
	
	m_pViewport->End();
	
	pRender->EndScene();
}

