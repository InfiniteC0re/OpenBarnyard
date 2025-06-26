#pragma once
#include "Render/ARenderer.h"
#include "AGUIMatlibPicture.h"

#include <Plugins/GUI/PGUITRRenderObject.h>
#include <Toshi/TPString8.h>

class AGUISystem : public Toshi::TTask
    , public Toshi::TSingleton<AGUISystem>
{
public:
	TDECLARE_CLASS( AGUISystem, Toshi::TTask );
	static constexpr TUINT NUM_RENDER_OBJECTS = 2;

public:
	AGUISystem();

	virtual TBOOL OnCreate() override;
	virtual void  OnDestroy() override;

	void SetPicture( const Toshi::TPString8& a_rName );
	void ResetPicture();

	PGUITRRenderObject* GetRenderObject( TUINT a_uiIndex )
	{
		TASSERT( a_uiIndex < NUM_RENDER_OBJECTS );
		return m_pRenderObjects[ a_uiIndex ];
	}

	AGUIMatlibPicture& GetMatlibPicture()
	{
		return m_MatLibPicture;
	}

public:
	static void AllowBackgroundClear( TBOOL a_bAllow );

private:
	static TBOOL __stdcall OnGUIRender( AGUISystem* a_pGUISystem, ARenderer* a_pRenderer, TINT* a_pUnknown );

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
