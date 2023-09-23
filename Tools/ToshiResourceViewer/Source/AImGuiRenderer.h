#pragma once
#include <Toshi/Render/TViewport.h>
#include <Toshi/Utils/TSingleton.h>
#include <Toshi/Render/TCameraObject.h>

#ifdef TOSHI_RENDERER_OPENGL
#include <Platform/SDL/TRender_SDL.h>
#endif

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
#ifdef TOSHI_RENDERER_OPENGL
	Toshi::TGLShaderProgram m_ShaderProgram;
	Toshi::TGLVertexArrayRef m_VertexArray;
#endif
	Toshi::TCameraObject* m_pCameraObject;
};
