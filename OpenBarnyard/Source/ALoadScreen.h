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

	static constexpr TUINT32 NUM_LOAD_TEXTURES = sizeof(ms_LoadTextures) / sizeof(*ms_LoadTextures);

public:
	ALoadScreen();
	virtual ~ALoadScreen();

	void Create();
	void StartLoading(TINT a_iUnused, TBOOL a_bRender);
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
	TINT m_iCowIndex;
	TINT m_iCowPosIndex;
	AGUI2Rectangle m_CanvasElement;
	AGUI2Rectangle m_aRectangles[9];
	AGUI2TextureSection* m_aTextureSections[NUM_LOAD_TEXTURES];
	// ...
	AGUI2TextBox m_TextBox;
	Toshi::THPTimer m_oTimer;
	// ...
	TFLOAT m_fTimer;
	TFLOAT m_fTotalTime;
};
