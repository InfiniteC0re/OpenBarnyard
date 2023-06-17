#ifdef TOSHI_DEBUG

#pragma once
#include "imgui.h"

class AImGui :
	public Toshi::TSingleton<AImGui>
{
public:
	AImGui();

	void Render();

private:
	void BeginScene();
	void EndScene();

private:
	TBOOL m_bIsInFrame;
};

#endif // TOSHI_DEBUG