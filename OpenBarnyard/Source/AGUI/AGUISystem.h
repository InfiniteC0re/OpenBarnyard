#pragma once
#include "Render/ARenderer.h"

#include <Plugins/GUI/PGUITRRenderObject.h>
#include <Toshi/Strings/TPString8.h>
#include <Toshi2/T2SimpleArray.h>

TOBJECT(AGUISystem, Toshi::TTask, TTRUE),
public Toshi::TSingleton<AGUISystem>
{
public:
	static constexpr TUINT NUM_RENDER_OBJECTS = 2;

public:
	AGUISystem();

	virtual TBOOL OnCreate() override;
	virtual void OnDestroy() override;

	PGUITRRenderObject* GetRenderObject(TUINT a_uiIndex)
	{
		TASSERT(a_uiIndex < NUM_RENDER_OBJECTS);
		return m_pRenderObjects[a_uiIndex];
	}

private:
	static TBOOL OnGUIRender(AGUISystem* a_pGUISystem, ARenderer* a_pRenderer, TINT* a_pUnknown);

private:
	TUINT m_Unk1;
	Toshi::TGenericListener m_SomeListener;
	Toshi::TListener<ARenderer, TINT, AGUISystem> m_RenderGUIListener;
	Toshi::T2SimpleArray<Toshi::TPString8> m_pStrings;
	TUINT m_Unk2;
	TUINT m_Unk3;
	PGUITRRenderObject* m_pRenderObjects[NUM_RENDER_OBJECTS];
};
