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

#ifdef BARNYARD_LATEST
#  include "GUI/XUI/AGUI2XUI.h"
class AGUI2XUIResource;
class AGUI2XUIScene;
class AGUI2XUICanvas;
#endif // BARNYARD_LATEST

class AGUI2
    : public Toshi::TTask
    , public Toshi::TSingleton<AGUI2>
{
public:
	TDECLARE_CLASS( AGUI2, Toshi::TTask );

#ifdef BARNYARD_LATEST
	// The Wii screen (root element) size the XUI scenes are made for
	static constexpr TFLOAT XUI_SCREEN_WIDTH  = 640.0f;
	static constexpr TFLOAT XUI_SCREEN_HEIGHT = 480.0f;
#endif

public:
	AGUI2();
	~AGUI2();

	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;
	virtual void  OnDestroy() override;

	void GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight );

	TBOOL IsCursorVisible() const { return m_bShowMouseCursor; }
	void  SetCursorVisible( TBOOL a_bVisible ) { m_bShowMouseCursor = a_bVisible; }

#ifdef BARNYARD_LATEST
	//-----------------------------------------------------------------------------
	// XUI frontend of the Wii build
	//-----------------------------------------------------------------------------
	void           AddXUIScene( AGUI2XUIScene* a_pScene );
	void           RemoveXUIScene( AGUI2XUIScene* a_pScene );
	TBOOL          LoadXUIResource( const TCHAR* a_szName );
	AGUI2XUIScene* CreateXUIScene( AGUI2XUIResource* a_pResource, TUINT32 a_uiIndex );
	void           UnloadXUIMaterialLibraries();
	void           CloseXUIFonts();
	void           LoadXUIMaterialLibrary( const TCHAR* a_szName );
	void           LoadXUIFont( const TCHAR* a_szName );
	void           DeleteXUIScenes();
	void           DeleteXUIResources();

	AGUI2XUI&       GetXUI() { return m_oXUI; }
	AGUI2XUICanvas* GetXUICanvas() const { return m_pXUICanvas; }

	static const TCHAR* GetLanguage();

#  ifdef BARNYARD_COMMUNITY_PATCH
	void DevLoadXUIScene( const TCHAR* a_szArgs );
#  endif
#endif // BARNYARD_LATEST

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

#ifdef BARNYARD_LATEST
	// NOTE: the Wii AGUI2 has a different layout, these are at 0xC4C-0xC9C there
	AGUI2XUI        m_oXUI;             // 0xC4C
	AGUI2XUIScene*  m_pFocusedXUIScene; // 0xC88
	void*           m_pXUIUnk1;         // 0xC8C
	void*           m_pXUIUnk2;         // 0xC90
	void*           m_pXUIUnk3;         // 0xC94
	AGUI2XUICanvas* m_pXUICanvas;       // 0xC98

	// Wii: layers over the whole screen (0x28 and 0xD0) and over the screen without a 16 pixel
	// border (0x7C, the XUI canvas is in this one)
	Toshi::T2GUIElement m_oLayer1;
	Toshi::T2GUIElement m_oSafeLayer;
	Toshi::T2GUIElement m_oLayer3;
#endif // BARNYARD_LATEST
};

TSINGLETON_DECLARE_ALIAS( AGUI2, GUI2 );
