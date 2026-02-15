#pragma once
#include "AGUI2Style.h"
#include "AGUI2TextBox.h"
#include "AGUI2MouseCursor.h"

#include <GUI/T2GUI.h>
#include <GUI/T2GUIContext.h>
#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIRenderer.h>
#include <GUI/T2GUITextureSection.h>

#include <Toshi/TTask.h>

class AGUI2
	: public Toshi::TTask
    , public Toshi::TSingleton<AGUI2>
{
public:
	TDECLARE_CLASS( AGUI2, Toshi::TTask );

public:
	AGUI2();
	~AGUI2();

	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;

	void GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight );

	TBOOL IsCursorVisible() const { return m_bShowMouseCursor; }
	void  SetCursorVisible( TBOOL a_bVisible ) { m_bShowMouseCursor = a_bVisible; }

public:
	static AGUI2MouseCursor&     GetMouseCursor() { return GetSingletonSafe()->m_oMouseCursor; }
	static Toshi::T2GUIElement*  GetRootElement() { return GetSingletonSafe()->m_pRootElement; }
	static Toshi::T2GUIContext*  GetContext() { return Toshi::T2GUI::GetContext(); }
	static Toshi::T2GUIRenderer* GetRenderer() { return Toshi::T2GUI::GetRenderer(); }

private:
	static void MainPostRenderCallback();

private:
	Toshi::T2GUIElement* m_pRootElement;
	AGUI2MouseCursor     m_oMouseCursor;
	TBOOL                m_bShowMouseCursor;
	AGUI2TextBox         m_oFPS;
	AGUI2TextBox         m_oMemStats;
	AGUI2TextBox         m_oPlayerInfo;
	AGUI2TextBox         m_oTexturesInfo;
	TWCHAR               m_wszMemStats[ 320 ];
	TWCHAR               m_wszFPS[ 64 ];
	TWCHAR               m_wszPlayerInfo[ 128 ];
	TWCHAR               m_wszTexturesInfo[ 64 ];
	TBOOL                m_bShowMemStatsInfo;
	TBOOL                m_bShowFPSInfo;
	TBOOL                m_bShowPlayerInfo;
	TBOOL                m_bShowTexturesInfo;
};

TSINGLETON_DECLARE_ALIAS( AGUI2, GUI2 );
