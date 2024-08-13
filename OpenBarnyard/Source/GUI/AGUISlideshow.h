#pragma once
#include "AGUI2Rectangle.h"
#include "GameInterface/AGameState.h"

#include <Toshi/TPString8.h>
#include <ToshiTools/T2DynamicVector.h>

class AGUISlideshow : public Toshi::TObject
{
public:
	TDECLARE_CLASS( AGUISlideshow, Toshi::TObject );

	using Flags = TUINT32;
	enum Flags_ : Flags
	{
		Flags_None			 = 0,
		Flags_Skippable		 = BITFLAG( 0 ),
		Flags_Unk1			 = BITFLAG( 1 ),
		Flags_InstaSkippable = BITFLAG( 2 ),
		Flags_HasFadeIn		 = BITFLAG( 3 ),
		Flags_HasFadeOut	 = BITFLAG( 4 ),
		Flags_Repeat		 = BITFLAG( 5 ),
		Flags_Ended			 = BITFLAG( 6 ),
	};

	struct Params
	{
		TINT   iUnk1;
		TBOOL  bSlideSkippable;
		TBOOL  bUnk2;
		TBOOL  bInstaSkippable;
		TBOOL  bHasFadeIn;
		TBOOL  bHasFadeOut;
		TBOOL  bRepeat;
		TBOOL  bDelayAppear;
		TFLOAT fDuration;
		TFLOAT fFadeInTime;
		TFLOAT fFadeOutTime;
		TFLOAT fUnk5;
	};

public:
	AGUISlideshow();

	TBOOL Setup( AGameState::HUDParams* a_pHUDParams, const Params& a_rParams, TBOOL a_bShouldLocalise );
	void  Update( TFLOAT a_fDeltaTime );
	void  UpdateFadeOverlay();
	void  SwitchToNextSlide( TBOOL a_bUnused = TTRUE );

	void Activate();
	void Deactivate();
	void Reset();

	TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pEvent );
	TBOOL ProcessCommand( AInputCommand a_eCommand );
	TBOOL IsSlideshowOver();

	TBOOL HasSlides() const
	{
		return m_Images.Size() != 0;
	}

	void AddSlide( const Toshi::TPString8& a_rName )
	{
		m_Images.PushBack( a_rName );
	}

public:
	static Toshi::TPString8* LocaliseBackgroundFileName( Toshi::TPString8& a_rOutName, const Toshi::TPString8& a_rName );
	static void				 LocaliseBackgroundFileName( Toshi::TPString8& a_rOutName, const TCHAR* a_szName );

protected:
	AGameState::HUDParams*							   m_pHUDParams;
	Toshi::T2DynamicVector<Toshi::TPString8>		   m_Images;
	Toshi::T2DynamicVector<Toshi::TPString8>::Iterator m_ImageIterator;
	Flags											   m_eFlags;
	TBOOL											   m_bIsActive;
	TFLOAT											   m_fCurrentSlideTime;
	TFLOAT											   m_fUnk2;
	TFLOAT											   m_fDuration;
	TFLOAT											   m_fFadeInTime;
	TFLOAT											   m_fFadeOutTime;
	TFLOAT											   m_fUnk4;
	AGUI2Rectangle									   m_FadeOverlay;
	TBOOL											   m_bIsAppearing;
	TBOOL											   m_bShouldLocalise;
};
