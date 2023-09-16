#ifdef TOSHI_DEBUG

#pragma once
#include "imgui.h"

#define AIMGUI_FORMAT(FORMAT, ...) ImGui::Text(FORMAT, __VA_ARGS__)
#define AIMGUI_STRING(FIELDNAME) ImGui::Text(#FIELDNAME": %s", FIELDNAME)
#define AIMGUI_DOUBLE(FIELDNAME) ImGui::Text(#FIELDNAME": %lf", FIELDNAME)
#define AIMGUI_FLOAT(FIELDNAME) ImGui::Text(#FIELDNAME": %f", FIELDNAME)
#define AIMGUI_INT(FIELDNAME) ImGui::Text(#FIELDNAME": %d", FIELDNAME)
#define AIMGUI_BOOL(FIELDNAME) ImGui::Text(#FIELDNAME": %d", FIELDNAME)

class AImGui :
#ifdef TOSHI_RENDERER_OPENGL
	public Toshi::TSingleton<AImGui>,
	Toshi::TSDLWindowListener
#else 
	public Toshi::TSingleton<AImGui>
#endif
{
public:
	AImGui();

	void Render();

#ifdef TOSHI_RENDERER_OPENGL
	virtual TBOOL OnEvent(const SDL_Event& a_rEvent);
#endif

private:
	void BeginScene();
	void EndScene();

private:
	TBOOL m_bIsInFrame;
};

#endif // TOSHI_DEBUG