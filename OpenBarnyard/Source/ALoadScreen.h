#pragma once
#include "GUI/AGUI2TextBox.h"

#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUITextureSection.h>

#include <Toshi/THPTimer.h>

#ifdef BARNYARD_LATEST
class AGUI2XUIScene;
#endif

class MODAPI ALoadScreen
{
public:
	static constexpr const TCHAR* ms_LoadTextures[] = {
		"Loader_Cow_01",
		"Loader_Cow_02",
	};

	static constexpr TUINT32 NUM_LOAD_TEXTURES    = TARRAYSIZE( ms_LoadTextures );
	static constexpr TUINT32 NUM_LOAD_INDICATORS  = 9;
	static constexpr TFLOAT  ICON_SWITCH_INTERVAL = 0.4f;

public:
	ALoadScreen();
	virtual ~ALoadScreen();

	void Create();

#ifdef BARNYARD_COMMUNITY_PATCH
	// Shows the "Loading..." text of the current language (AFrontEndLanguageState). Not on PC
	void UpdateLoadingText();
#endif
	void StartLoading( TINT a_iUnused, TBOOL a_bRender );
	void SetLoadingState( TBOOL a_bIsLoading, TBOOL a_bUpdateUI );
	void Update( TFLOAT a_fDeltaTime = 1.0f, TBOOL a_bRender = TTRUE );
	void ResetIndicators( TBOOL a_bRender );

	// Shows a_rcPicture as the background of the loading screen (does nothing if it's empty)
	void SetPicture( const Toshi::TPString8& a_rcPicture, TBOOL a_bRender );

	void Reset();

	// Detaches the elements of the load screen from the GUI
	void RemoveElements();

#ifdef BARNYARD_LATEST
	// Wii: shows the scene a_uiIndex of the loaded XUI resource a_szName as the loading screen
	AGUI2XUIScene* SetXUIScene( const TCHAR* a_szName, TUINT32 a_uiIndex );

	// Wii: draws AGUISystem's background picture instead of the black canvas (the backdrop of the XUI scenes)
	void ShowXUIBackground( TBOOL a_bShow, TBOOL a_bRender );

	void DeleteXUIScene();

	AGUI2XUIScene* GetXUIScene() const { return m_pXUIScene; }
#endif

	TBOOL IsLoadingScreen() const
	{
		return m_bIsLoadingScreen;
	}

private:
	void UpdateUI( TBOOL a_bRender );

private:
	// ...
	TBOOL                       m_bIsLoadingScreen;
	TBOOL                       m_bFlag;
	TINT                        m_iNumCowIcons;
	TINT                        m_iCowPos;
	Toshi::T2GUIRectangle       m_CanvasElement;
	Toshi::T2GUIRectangle       m_aRectangles[ NUM_LOAD_INDICATORS ];
	Toshi::T2GUITextureSection* m_aTextureSections[ NUM_LOAD_TEXTURES ];
	// ...
	AGUI2TextBox    m_TextBox;
	Toshi::THPTimer m_oTimer;
	// ...
	TFLOAT m_fTimer;
	TFLOAT m_fTotalTime;

#ifdef BARNYARD_LATEST
	AGUI2XUIScene* m_pXUIScene; // 0x580 (Wii)
#endif
};

extern ALoadScreen g_oLoadScreen;
