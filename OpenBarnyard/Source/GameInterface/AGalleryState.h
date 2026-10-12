#pragma once
#include "AGameState.h"
#include "GUI/T2GUIRectangle.h"
#include "GUI/AGUI2TextBox.h"

//-----------------------------------------------------------------------------
// Shows the concept art pictures of a category of the art gallery as the
// background picture of AGUISystem (TClass 0077e1bc, vtable 007175e0).
// Left/Right browse the pictures, OK/Cancel go back.
//-----------------------------------------------------------------------------
class AGalleryState : public AGameState
{
public:
	TDECLARE_CLASS( AGalleryState, AGameState );

	static constexpr TINT NUM_CATEGORIES = 9;

	enum ARROW
	{
		ARROW_LEFT,
		ARROW_RIGHT,
		ARROW_NUMOF
	};

public:
	AGalleryState( TINT a_iCategory = 0 );
	~AGalleryState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;
	virtual void  OnDeactivate() OVERRIDE;

private:
	// Shows the picture a_iPicture of the category a_iCategory and its number
	void ShowPicture( TINT a_iCategory, TINT a_iPicture );

private:
	// 0x00716fb0
	inline static constexpr TINT ms_aNumPictures[ NUM_CATEGORIES ] = { 6, 6, 6, 7, 12, 6, 9, 7, 6 };
	// 0x00716fd4
	inline static constexpr TINT ms_aCategoryNames[ NUM_CATEGORIES ] = { 112, 113, 114, 115, 116, 117, 118, 119, 85 };
	// 0x00772760
	static const TCHAR* const* const ms_aPictures[ NUM_CATEGORIES ];

private:
	TINT                  m_iCategory;              // 0xa4
	TINT                  m_iPicture;               // 0xa8
	TWCHAR                m_wszPage[ 16 ];          // 0xac
	AGUI2TextBox          m_oTitle;                 // 0xcc, NOTE: never added to the screen
	AGUI2TextBox          m_oPage;                  // 0x144
	Toshi::T2GUIRectangle m_aArrows[ ARROW_NUMOF ]; // 0x1bc
};
