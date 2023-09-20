#pragma once
#include <Toshi/Render/TViewport.h>
#include <Toshi/Utils/TSingleton.h>
#include <Toshi/Render/TCameraObject.h>
#include <Platform/SDL/TRender_SDL.h>

class AImGuiRenderer : public Toshi::TSingleton<AImGuiRenderer>
{
public:
	AImGuiRenderer();

	TBOOL CreateTRender();
	TBOOL CreateMainViewport();
	TBOOL CreateImGui();

	void Update(float a_fDeltaTime);

private:
	Toshi::TViewport* m_pViewport;
	Toshi::TGLShaderProgram m_ShaderProgram;
	Toshi::TGLVertexArrayRef m_VertexArray;
	Toshi::TCameraObject* m_pCameraObject;
};
