#pragma once
#include "AGameState.h"
#include "GUI/AGUI2TextBox.h"
#include "GUI/AGUI2ModelRenderer.h"

#include <GUI/T2GUIRectangle.h>
#include <Math/TMatrix44.h>

class AAnimatableObject;
class AUnitType;

//-----------------------------------------------------------------------------
// The reward screen (TClass_ABarnRewardState, vtable 0071a588): a spinning model
// of the reward on the wooden background with flares behind it, its name (or the
// amount of money) and an OK prompt. Pushed by AGiftActivity ("showreward") for
// the special items and by APlayerSimProfileActivity for the money.
// Some rewards unlock a sim loading screen picture, it's shown on a second page
// after OK is pressed (once per picture).
//-----------------------------------------------------------------------------
class ABarnRewardState
    : public AGameState
{
public:
	TDECLARE_CLASS( ABarnRewardState, AGameState );

	using TYPE = TINT;
	enum TYPE_ : TYPE
	{
		TYPE_ITEM  = 0, // shows the screen name of the item
		TYPE_TOKEN = 1, // same as TYPE_ITEM (goldrush token)
		TYPE_MONEY = 2, // shows the amount of money (locale 355), the model is "buck_1"
	};

	using PAGE = TINT;
	enum PAGE_ : PAGE
	{
		PAGE_REWARD  = 0,
		PAGE_PICTURE = 1, // the unlocked sim loading picture
	};

	// A flare behind the model, pulsing with its own frequency
	struct Flare
	{
		// NOTE: folded with the constructor of AGUI2MiniMap::Icon (it also only constructs the rectangle)
		// $Barnyard: FUNCTION 004311e0
		Flare() {}

		Toshi::T2GUIRectangle oRectangle; // 0x00
		TFLOAT                fTime;      // 0x78
		TFLOAT                fFrequency; // 0x7c
	};

public:
	// Money reward
	ABarnRewardState( TYPE a_eType, TINT a_iMoney );

	// Item reward, a_iPicture is the sim loading picture it unlocks (-1 if none)
	ABarnRewardState( TYPE a_eType, const Toshi::TPString8& a_rcItemType, TINT a_iPicture );

	~ABarnRewardState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	// RETURNS_FALSE 006ce000
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE { return TFALSE; }                                      // +0x0c
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // +0x10
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x4c
	virtual void  OnInsertion() OVERRIDE;                                                                                                                  // +0x50
	virtual void  OnActivate() OVERRIDE;                                                                                                                   // +0x60
	virtual void  OnDeactivate() OVERRIDE;                                                                                                                 // +0x64

private:
	TYPE                m_eType;            // 0xa4
	AAnimatableObject*  m_pObject;          // 0xa8
	AUnitType*          m_pUnitType;        // 0xac
	TINT                m_iMoney;           // 0xb0, -1 for items
	Toshi::TMatrix44    m_oModelTransform;  // 0xb4, only the translation is set (OnInsertion), never used
	Flare               m_aFlares[ 4 ];     // 0xf4
	Toshi::T2GUIElement m_oFrame;           // 0x2f4
	Toshi::T2GUIElement m_oPicturePage;     // 0x348
	AGUI2TextBox        m_oTitle;           // 0x39c
	AGUI2TextBox        m_oSubtitle;        // 0x414
	AGUI2TextBox        m_oName;            // 0x48c
	TWCHAR              m_wszMoney[ 64 ];   // 0x504
	AGUI2TextBox        m_oPressOK;         // 0x584
	TWCHAR              m_wszPressOK[ 64 ]; // 0x5fc
	TFLOAT              m_fScreenSize;      // 0x67c, the biggest of the screen width and height
	TFLOAT              m_fRotation;        // 0x680
	AGUI2ModelRenderer  m_oModelRenderer;   // 0x684
	PAGE                m_ePage;            // 0x704
	TINT                m_iPicture;         // 0x708
};

TSTATICASSERT( sizeof( ABarnRewardState ) == 0x70c );
