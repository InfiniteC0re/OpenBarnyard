#pragma once
#include "SDK_T2GUIContext.h"
#include "SDK_T2GUIElement.h"
#include "AGUI2Renderer.h"
#include "AGUI2TextBox.h"
#include "AGUI2MouseCursor.h"
#include "THookedObject.h"
#include "THookedSingleton.h"

#include <Toshi/TTask.h>

class AGUI2 : public Toshi::TTask
    , public THookedSingleton<AGUI2, 0x007b4ff4>
{
public:
	virtual TBOOL OnCreate()                      = 0;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) = 0;

	void GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight );

	TBOOL IsCursorVisible() const
	{
		return m_bShowMouseCursor;
	}

public:
	static SDK_T2GUIElement*  GetRootElement() { return GetSingletonSafe()->m_pRootElement; }
	static SDK_T2GUIContext*  GetContext() { return *ms_ppCurrentContext; }
	static SDK_T2GUIRenderer* GetRenderer();

private:
	inline static SDK_T2GUIContext** ms_ppCurrentContext = (SDK_T2GUIContext**)0x007cef20;

public:
	SDK_T2GUIElement*    m_pRootElement;
	AGUI2MouseCursor m_oMouseCursor;
	TBOOL            m_bShowMouseCursor;
	AGUI2TextBox     m_oFPS;
	AGUI2TextBox     m_oMemStats;
	AGUI2TextBox     m_oPlayerInfo;
	AGUI2TextBox     m_oTexturesInfo;
	wchar_t          m_wszMemStats[ 320 ];
	wchar_t          m_wszFPS[ 64 ];
	wchar_t          m_wszPlayerInfo[ 128 ];
	wchar_t          m_wszTexturesInfo[ 64 ];
	TBOOL            m_bShowMemStatsInfo;
	TBOOL            m_bShowFPSInfo;
	TBOOL            m_bShowPlayerInfo;
	TBOOL            m_bShowTexturesInfo;
};
