#pragma once
#include "GUI/AGUI2Rectangle.h"
#include "GUI/AGUI2TextBox.h"
#include "GUI/AGUI2TextureSection.h"

#include <Toshi/Core/THPTimer.h>

class ALoadScreen
{
public:
	static constexpr const char* ms_LoadTextures[] = {
		"Loader_Cow_01",
		"Loader_Cow_02",
	};

	static constexpr TUINT32 NUM_LOAD_TEXTURES = TARRAYSIZE(ms_LoadTextures);
	static constexpr TUINT32 NUM_LOAD_INDICATORS = 9;
	static constexpr TFLOAT ICON_SWITCH_INTERVAL = 0.4f;

public:
	ALoadScreen();
	virtual ~ALoadScreen();

	void Create();
	void StartLoading(TINT a_iUnused, TBOOL a_bRender);
	void SetLoadingState(TBOOL a_bIsLoading, TBOOL a_bUpdateUI);
	void Update(TFLOAT a_fDeltaTime = 1.0f, TBOOL a_bRender = TTRUE);

	static ALoadScreen* GetGlobalInstance()
	{
		static ALoadScreen s_Instance;
		return &s_Instance;
	}

private:
	void UpdateUI(TBOOL a_bRender);

private:
	// ...
	TBOOL m_bIsLoadingScreen;
	TBOOL m_bFlag;
	TINT m_iNumCowIcons;
	TINT m_iCowPos;
	AGUI2Rectangle m_CanvasElement;
	AGUI2Rectangle m_aRectangles[NUM_LOAD_INDICATORS];
	AGUI2TextureSection* m_aTextureSections[NUM_LOAD_TEXTURES];
	// ...
	AGUI2TextBox m_TextBox;
	Toshi::THPTimer m_oTimer;
	// ...
	TFLOAT m_fTimer;
	TFLOAT m_fTotalTime;
};
