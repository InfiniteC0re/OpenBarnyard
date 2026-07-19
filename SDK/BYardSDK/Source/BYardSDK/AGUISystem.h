#pragma once
#include "ARenderer.h"
#include "THookedSingleton.h"

#include <Plugins/GUI/PGUITRRenderObject.h>
#include <Toshi/TPString8.h>
#include <Toshi/TTask.h>

class AGUIMatlibPicture
{
	TCHAR PADDING[40];
};

class AGUISystem
    : public Toshi::TTask
    , public THookedSingleton<AGUISystem, 0x0078a738>
{
public:
	TDECLARE_CLASS( AGUISystem, Toshi::TTask );
	static constexpr TUINT NUM_RENDER_OBJECTS = 2;

public:
	AGUISystem() = delete;

	PGUITRRenderObject* GetRenderObject( TUINT a_uiIndex )
	{
		TASSERT( a_uiIndex < NUM_RENDER_OBJECTS );
		return m_pRenderObjects[ a_uiIndex ];
	}

private:
	TUINT                                         m_Unk1;
	Toshi::TGenericListener                       m_SomeListener;
	Toshi::TListener<ARenderer, TINT, AGUISystem> m_RenderGUIListener;
	Toshi::TPString8*                             m_pStrings;
	AGUIMatlibPicture                             m_MatLibPicture;
	TUINT                                         m_Unk2;
	TUINT                                         m_Unk3;
	PGUITRRenderObject*                           m_pRenderObjects[ NUM_RENDER_OBJECTS ];
};

TSINGLETON_DECLARE_ALIAS( AGUISystem, GUISystem );
