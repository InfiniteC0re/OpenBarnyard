#pragma once
#include "AGUI2Rectangle.h"
#include "GameInterface/AGameState.h"

#include <Toshi/Strings/TPString8.h>
#include <Toshi2/T2DynamicVector.h>

TOBJECT(AGUISlideshow, Toshi::TObject, TTRUE)
{
public:
	using Flags = TUINT32;
	enum Flags_ : Flags
	{
		Flags_None = 0,
		Flags_Skippable = BITFIELD(0),
		Flags_Unk1 = BITFIELD(1),
		Flags_InstaSkippable = BITFIELD(2),
		Flags_HasFade = BITFIELD(3),
		Flags_Unk2 = BITFIELD(4),
		Flags_Repeat = BITFIELD(5),
		Flags_Ended = BITFIELD(6),
	};

	struct Params
	{
		TINT iUnk1;
		TBOOL bSlideSkippable;
		TBOOL bUnk2;
		TBOOL bInstaSkippable;
		TBOOL bHasFade;
		TBOOL bUnk3;
		TBOOL bRepeat;
		TBOOL bIsFading;
		TFLOAT fDuration;
		TFLOAT fFadeTime;
		TFLOAT fUnk4;
		TFLOAT fUnk5;
	};

public:
	AGUISlideshow();

	TBOOL Setup(AGameState::HUDParams* a_pHUDParams, const Params& a_rParams, TBOOL a_bFlag);
	void Activate();
	void Update(TFLOAT a_fDeltaTime);
	void UpdateFadeOverlay();
	void SwitchToNextSlide();
	void EndSlideshow();
	void ResetSlideshow();

	TBOOL ProcessInput(const Toshi::TInputInterface::InputEvent* a_pEvent);
	TBOOL ProcessCommand(AInputCommand a_eCommand);
	TBOOL IsSlideshowOver();

	void AddSlide(const Toshi::TPString8& a_rName)
	{
		m_Images.PushBack(a_rName);
	}

public:
	static Toshi::TPString8* LocaliseBackgroundFileName(Toshi::TPString8& a_rOutName, const Toshi::TPString8& a_rName);
	static void LocaliseBackgroundFileName(Toshi::TPString8& a_rOutName, const char* a_szName);

private:
	AGameState::HUDParams* m_pHUDParams;
	Toshi::T2DynamicVector<Toshi::TPString8> m_Images;
	Toshi::T2DynamicVector<Toshi::TPString8>::Iterator m_ImageIterator;
	Flags m_eFlags;
	TBOOL m_bIsActive;
	TFLOAT m_fCurrentSlideTime;
	TFLOAT m_fUnk2;
	TFLOAT m_fDuration;
	TFLOAT m_fFadeTime;
	TFLOAT m_fUnk3;
	TFLOAT m_fUnk4;
	AGUI2Rectangle m_FadeOverlay;
	TBOOL m_bIsFading;
	TBOOL m_bFlag;
};
