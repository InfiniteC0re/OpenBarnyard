#pragma once
#include "AGUI2Context.h"
#include "AGUI2Element.h"
#include "AGUI2Renderer.h"
#include "AGUI2TextBox.h"
#include "AGUI2MouseCursor.h"
#include "AGUI2TextureSection.h"
#include "THookedObject.h"
#include "THookedSingleton.h"

#include <Toshi/TTask.h>

class AGUI2 :
	public Toshi::TTask,
	public THookedSingleton<AGUI2, 0x007b4ff4>
{
public:
	virtual TBOOL OnCreate()					  = 0;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) = 0;

	void GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight );

	TBOOL IsCursorVisible() const
	{
		return m_bShowMouseCursor;
	}

public:
	static AGUI2Element*  GetRootElement() { return GetSingletonSafe()->m_pRootElement; }
	static AGUI2Context*  GetContext() { return *ms_ppCurrentContext; }
	static AGUI2Renderer* GetRenderer();

private:
	inline static AGUI2Context** ms_ppCurrentContext = (AGUI2Context**)0x007cef20;

public:
	AGUI2Element*	 m_pRootElement;
	AGUI2MouseCursor m_oMouseCursor;
	TBOOL			 m_bShowMouseCursor;
	AGUI2TextBox	 m_oFPS;
	AGUI2TextBox	 m_oMemStats;
	AGUI2TextBox	 m_oPlayerInfo;
	AGUI2TextBox	 m_oTexturesInfo;
	wchar_t			 m_wszMemStats[ 320 ];
	wchar_t			 m_wszFPS[ 64 ];
	wchar_t			 m_wszPlayerInfo[ 128 ];
	wchar_t			 m_wszTexturesInfo[ 64 ];
	TBOOL			 m_bShowMemStatsInfo;
	TBOOL			 m_bShowFPSInfo;
	TBOOL			 m_bShowPlayerInfo;
	TBOOL			 m_bShowTexturesInfo;
};
