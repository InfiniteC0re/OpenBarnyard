#pragma once
#include "ABYardMenuState.h"
#include "GUI/AGUI2Button.h"

//-----------------------------------------------------------------------------
// The Bonuses menu of the frontend (TClass 0077df7c, vtable 00716568).
// MODE_BONUSES: art gallery, movie gallery and credits.
// MODE_TRAILERS: the two trailers (nothing opens this mode in the PC retail).
//-----------------------------------------------------------------------------
class MODAPI AFrontEndBonusState : public ABYardMenuState
{
public:
	TDECLARE_CLASS( AFrontEndBonusState, ABYardMenuState );

	using MODE = TINT;
	enum MODE_ : MODE
	{
		MODE_BONUSES,
		MODE_TRAILERS,
	};

	static constexpr TINT MAX_NUM_BUTTONS = 3;

	enum BONUSBUTTON
	{
		BONUSBUTTON_ARTGALLERY,
		BONUSBUTTON_MOVIEGALLERY,
		BONUSBUTTON_CREDITS,
	};

public:
	AFrontEndBonusState( MODE a_eMode );
	~AFrontEndBonusState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;

	//-----------------------------------------------------------------------------
	// ABYardMenuState
	//-----------------------------------------------------------------------------
	virtual void OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem ) OVERRIDE;
	virtual void OnMenuClose() OVERRIDE;

private:
	TINT GetNumButtons() const { return ( m_eMode != MODE_BONUSES ) ? 2 : 3; }

private:
	// 0x00716554
	inline static constexpr TINT ms_aBonusTexts[ 3 ] = { 72, 73, 280 };
	// 0x00716560
	inline static constexpr TINT ms_aTrailerTexts[ 2 ] = { 1062, 1063 };
	// 0x00772634
	inline static const TCHAR* ms_aTrailerMovies[ 2 ] = { "Trailer1", "Trailer2" };

private:
	AGUI2Button m_aButtons[ MAX_NUM_BUTTONS ]; // 0x7d8
	TINT        m_iActivatedButtonID;          // 0xde4
	MODE        m_eMode;                       // 0xde8
};
